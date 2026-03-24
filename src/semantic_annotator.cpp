#include "semantic_annotator.h"

#include <cctype>
#include <string>
#include <iostream>
namespace {

std::string toLower(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string dataTypeToString(DataType t) {
    switch (t) {
    case DataType::Integer: return "integer";
    case DataType::Real: return "real";
    case DataType::Boolean: return "boolean";
    case DataType::Char: return "char";
    case DataType::Procedure: return "procedure";
    case DataType::Function: return "function";
    case DataType::Unknown: return "unknown";
    }
    return "unknown";
}

}  // namespace

SemanticAnnotator::SemanticAnnotator(SymbolTable& symbolTable, ErrorHandler& errorHandler)
    : symbolTable_(symbolTable), errorHandler_(errorHandler) {}

void SemanticAnnotator::annotate(ASTNode* root) {
    annotateNode(root);
}

void SemanticAnnotator::annotateValueNode(ASTNode* node) {
    ++valueContextDepth_;
    annotateNode(node);
    --valueContextDepth_;
}

bool SemanticAnnotator::isValueContext() const {
    return valueContextDepth_ > 0;
}

void SemanticAnnotator::annotateNode(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    switch (node->nodeType) {
    case NodeType::Program:      annotateProgram(static_cast<ProgramNode*>(node)); break;
    case NodeType::Block:        annotateBlock(static_cast<BlockNode*>(node)); break;
    case NodeType::List:         annotateList(static_cast<ListNode*>(node)); break;

    case NodeType::ConstDecl:    annotateConstDecl(static_cast<ConstDeclNode*>(node)); break;
    case NodeType::VarDecl:      annotateVarDecl(static_cast<VarDeclNode*>(node)); break;
    case NodeType::ParamDecl:    annotateParamDecl(static_cast<ParamDeclNode*>(node)); break;
    case NodeType::ProcDecl:     annotateProcDecl(static_cast<ProcDeclNode*>(node)); break;
    case NodeType::FuncDecl:     annotateFuncDecl(static_cast<FuncDeclNode*>(node)); break;

    case NodeType::AssignStmt:   annotateAssignStmt(static_cast<AssignStmtNode*>(node)); break;
    case NodeType::IfStmt:       annotateIfStmt(static_cast<IfStmtNode*>(node)); break;
    case NodeType::WhileStmt:    annotateWhileStmt(static_cast<WhileStmtNode*>(node)); break;
    case NodeType::ForStmt:      annotateForStmt(static_cast<ForStmtNode*>(node)); break;
    case NodeType::CompoundStmt: annotateCompoundStmt(static_cast<CompoundStmtNode*>(node)); break;
    case NodeType::ProcCall:     annotateProcCall(static_cast<ProcCallNode*>(node)); break;

    case NodeType::BinaryExpr:   annotateBinaryExpr(static_cast<BinaryExprNode*>(node)); break;
    case NodeType::UnaryExpr:    annotateUnaryExpr(static_cast<UnaryExprNode*>(node)); break;
    case NodeType::Identifier:   annotateIdentifier(static_cast<IdentifierNode*>(node)); break;
    case NodeType::Literal:      annotateLiteral(static_cast<LiteralNode*>(node)); break;
    case NodeType::ArrayAccess:  annotateArrayAccess(static_cast<ArrayAccessNode*>(node)); break;
    case NodeType::ArrayType:    annotateArrayType(static_cast<ArrayTypeNode*>(node)); break;
    }
}

void SemanticAnnotator::annotateProgram(ProgramNode* node) {
    // Program root has a mandatory Block child and an optional identifier list
    // from the program header (e.g., program p(input, output)).
    // The header identifier list is metadata and should not be treated as
    // ordinary variables in this Pascal-S subset.
    for (ASTNode* child : node->children) {
        if (child->nodeType == NodeType::Block) {
            annotateNode(child);
        }
    }
    // 注解AST时，程序头部的标识符列表不应被视为普通变量，因此不对其进行注解。
}

void SemanticAnnotator::annotateBlock(BlockNode* node) {
    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
}

void SemanticAnnotator::annotateList(ListNode* node) {
    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
}

void SemanticAnnotator::annotateConstDecl(ConstDeclNode* node) {
    if (node->children.size() < 2) {
        return;
    }

    ASTNode* idNode = node->children[0];
    ASTNode* valueNode = node->children[1];
    annotateNode(valueNode);

    IdentifierNode* id = dynamic_cast<IdentifierNode*>(idNode);
    if (id == nullptr) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Invalid const declaration: identifier expected");
        return;
    }

    SymbolEntry entry = SymbolEntry::makeConstant(id->identifier, inferType(valueNode));
    if (auto* literal = dynamic_cast<LiteralNode*>(valueNode)) {
        entry.hasConstLiteral = true;
        entry.constLiteralText = literal->value;
    }

    if (!symbolTable_.insert(entry)) {
        errorHandler_.report(id->pos.line, id->pos.col,
            "Redefinition of identifier: " + id->identifier);
        return;
    }

    id->symbolEntry = symbolTable_.lookup(id->identifier);
    id->dataType = entry.type;
    id->isLValue = false;
}

void SemanticAnnotator::annotateVarDecl(VarDeclNode* node) {
    if (node->children.size() < 2) {
        return;
    }

    ASTNode* idList = node->children[0];
    ASTNode* typeNode = node->children[1];
    DataType declaredType = inferTypeFromTypeNode(typeNode);
    const std::vector<ArrayBound> arrayBounds = collectArrayBounds(typeNode);

    auto declareOne = [&](IdentifierNode* id) {
        SymbolEntry entry = SymbolEntry::makeVariable(
            id->identifier,
            declaredType,
            typeNode->nodeType == NodeType::ArrayType // isArray
        );
        entry.arrayBounds = arrayBounds;

        if (!symbolTable_.insert(entry)) {
            errorHandler_.report(id->pos.line, id->pos.col,
                "Redefinition of identifier: " + id->identifier);
            return;
        }

        id->symbolEntry = symbolTable_.lookup(id->identifier);
        id->dataType = declaredType;
        id->isLValue = true;
    };

    if (auto* list = dynamic_cast<ListNode*>(idList)) {
        for (ASTNode* item : list->children) {
            if (auto* id = dynamic_cast<IdentifierNode*>(item)) {
                declareOne(id);
            }
        }
        return;
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(idList)) {
        declareOne(id);
    }
}

void SemanticAnnotator::annotateParamDecl(ParamDeclNode* node) {
    if (node->children.size() < 2) {
        return;
    }

    ASTNode* idList = node->children[0];
    ASTNode* typeNode = node->children[1];
    DataType declaredType = inferTypeFromTypeNode(typeNode);

    auto declareOne = [&](IdentifierNode* id) {
        SymbolEntry entry = SymbolEntry::makeParameter(id->identifier, declaredType, node->isVar);

        if (!symbolTable_.insert(entry)) {
            errorHandler_.report(id->pos.line, id->pos.col,
                "Redefinition of parameter: " + id->identifier);
            return;
        }

        id->symbolEntry = symbolTable_.lookup(id->identifier);
        id->dataType = declaredType;
        id->isLValue = true;
    };

    if (auto* list = dynamic_cast<ListNode*>(idList)) {
        for (ASTNode* item : list->children) {
            if (auto* id = dynamic_cast<IdentifierNode*>(item)) {
                declareOne(id);
            }
        }
        return;
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(idList)) {
        declareOne(id);
    }
}

void SemanticAnnotator::annotateProcDecl(ProcDeclNode* node) {
    SymbolEntry procEntry = SymbolEntry::makeProcedure(node->name);
    if (!node->children.empty()) {
        procEntry.params = collectParams(node->children[0]);
    }

    if (!symbolTable_.insert(procEntry)) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Redefinition of procedure: " + node->name);
        return;
    }

    symbolTable_.enterScope();
    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
    symbolTable_.exitScope();
}

void SemanticAnnotator::annotateFuncDecl(FuncDeclNode* node) {
    SymbolEntry funcEntry = SymbolEntry::makeFunction(node->name, node->retType);
    if (!node->children.empty()) {
        funcEntry.params = collectParams(node->children[0]);
    }

    if (!symbolTable_.insert(funcEntry)) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Redefinition of function: " + node->name);
        return;
    }

    functionContextStack_.push_back(toLower(node->name));
    // std::cout << "Entering function context: " << node->name << "\n";
    symbolTable_.enterScope();
    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
    symbolTable_.exitScope();
    functionContextStack_.pop_back();
    // std::cout << "Exiting function context: " << node->name << "\n";
}

void SemanticAnnotator::annotateAssignStmt(AssignStmtNode* node) {
    if (node->children.size() < 2) {
        return;
    }

    ASTNode* lhs = node->children[0];
    ASTNode* rhs = node->children[1];
    annotateNode(lhs);
    annotateValueNode(rhs);

    const bool functionResultAssign = isFunctionResultAssignment(lhs);

    if (!isLValue(lhs) && !functionResultAssign) {
        errorHandler_.report(lhs->pos.line, lhs->pos.col,
            "Left-hand side of assignment is not assignable");
    }

    DataType lhsType = inferType(lhs);
    DataType rhsType = inferType(rhs);
    if (!isAssignable(lhsType, rhsType)) {
        reportTypeMismatch(node, lhsType, rhsType, "assignment");
    }
}

void SemanticAnnotator::annotateIfStmt(IfStmtNode* node) {
    if (node->children.empty()) {
        return;
    }

    ASTNode* cond = node->children[0];
    annotateValueNode(cond);
    if (!isBooleanType(inferType(cond))) {
        reportTypeMismatch(cond, DataType::Boolean, inferType(cond), "if condition");
    }

    for (std::size_t i = 1; i < node->children.size(); ++i) {
        annotateNode(node->children[i]);
    }
}

void SemanticAnnotator::annotateWhileStmt(WhileStmtNode* node) {
    if (node->children.empty()) {
        return;
    }

    ASTNode* cond = node->children[0];
    annotateValueNode(cond);
    if (!isBooleanType(inferType(cond))) {
        reportTypeMismatch(cond, DataType::Boolean, inferType(cond), "while condition");
    }

    if (node->children.size() > 1) {
        annotateNode(node->children[1]);
    }
}

void SemanticAnnotator::annotateForStmt(ForStmtNode* node) {
    if (node->children.size() < 4) {
        return;
    }

    ASTNode* id = node->children[0];
    ASTNode* init = node->children[1];
    ASTNode* end = node->children[2];
    ASTNode* body = node->children[3];

    annotateNode(id);
    annotateValueNode(init);
    annotateValueNode(end);

    if (!isLValue(id) || !isIntegerType(inferType(id))) {
        errorHandler_.report(id->pos.line, id->pos.col,
            "For-loop variable must be an assignable integer");
    }
    if (!isIntegerType(inferType(init)) || !isIntegerType(inferType(end))) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "For-loop bounds must be integer expressions");
    }

    annotateNode(body);
}

void SemanticAnnotator::annotateCompoundStmt(CompoundStmtNode* node) {
    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
}

void SemanticAnnotator::annotateProcCall(ProcCallNode* node) {
    for (ASTNode* arg : node->children) {
        annotateValueNode(arg);
    }

    const SymbolEntry* entry = symbolTable_.lookup(node->name);
    if (entry == nullptr) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Undefined procedure/function: " + node->name);
        return;
    }

    if (entry->kind != SymbolKind::Procedure && entry->kind != SymbolKind::Function) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Symbol is not callable: " + node->name);
        return;
    }

    if (entry->kind == SymbolKind::Procedure && isValueContext()) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Procedure call cannot be used as a value: " + node->name);
    }

    node->symbolEntry = entry;
    node->dataType = entry->type;
    checkCallArguments(node, entry);

    node->isVarParam.clear();
    for (const ParamInfo& p : entry->params) {
        node->isVarParam.push_back(p.isVarParam);
    }
}

void SemanticAnnotator::annotateBinaryExpr(BinaryExprNode* node) {
    if (node->children.size() < 2) {
        node->dataType = DataType::Unknown;
        return;
    }

    ASTNode* lhs = node->children[0];
    ASTNode* rhs = node->children[1];
    annotateValueNode(lhs);
    annotateValueNode(rhs);

    DataType lt = inferType(lhs);
    DataType rt = inferType(rhs);
    const std::string op = toLower(node->op);

    if (op == "+" || op == "-" || op == "*") {
        if (!isNumericType(lt) || !isNumericType(rt)) {
            errorHandler_.report(node->pos.line, node->pos.col,
                "Arithmetic operator requires numeric operands");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = (lt == DataType::Real || rt == DataType::Real)
            ? DataType::Real
            : DataType::Integer;
        return;
    }

    if (op == "/") {
        if (!isNumericType(lt) || !isNumericType(rt)) {
            errorHandler_.report(node->pos.line, node->pos.col,
                "Division operator requires numeric operands");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = DataType::Real;
        return;
    }

    if (op == "div" || op == "mod") {
        if (!isIntegerType(lt) || !isIntegerType(rt)) {
            errorHandler_.report(node->pos.line, node->pos.col,
                "div/mod operator requires integer operands");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = DataType::Integer;
        return;
    }

    if (op == "and" || op == "or") {
        if (!isBooleanType(lt) || !isBooleanType(rt)) {
            errorHandler_.report(node->pos.line, node->pos.col,
                "Logical operator requires boolean operands");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = DataType::Boolean;
        return;
    }

    if (op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (!isAssignable(lt, rt) && !isAssignable(rt, lt)) {
            reportTypeMismatch(node, lt, rt, "relational expression");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = DataType::Boolean;
        return;
    }

    errorHandler_.report(node->pos.line, node->pos.col,
        "Unsupported binary operator: " + node->op);
    node->dataType = DataType::Unknown;
}

void SemanticAnnotator::annotateUnaryExpr(UnaryExprNode* node) {
    if (node->children.empty()) {
        node->dataType = DataType::Unknown;
        return;
    }

    ASTNode* expr = node->children[0];
    annotateValueNode(expr);

    DataType et = inferType(expr);
    const std::string op = toLower(node->op);
    if (op == "not") {
        if (!isBooleanType(et)) {
            reportTypeMismatch(node, DataType::Boolean, et, "unary not");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = DataType::Boolean;
        return;
    }

    if (op == "-" || op == "uminus") {
        if (!isNumericType(et)) {
            errorHandler_.report(node->pos.line, node->pos.col,
                "Unary minus requires numeric operand");
            node->dataType = DataType::Unknown;
            return;
        }
        node->dataType = et;
        return;
    }

    errorHandler_.report(node->pos.line, node->pos.col,
        "Unsupported unary operator: " + node->op);
    node->dataType = DataType::Unknown;
}

void SemanticAnnotator::annotateIdentifier(IdentifierNode* node) {
    const SymbolEntry* entry = symbolTable_.lookup(node->identifier);
    if (entry == nullptr) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Undefined identifier: " + node->identifier);
        node->dataType = DataType::Unknown;
        return;
    }

    node->symbolEntry = entry;
    node->dataType = entry->type;
    node->isLValue = entry->kind == SymbolKind::Variable || entry->kind == SymbolKind::Parameter;
}

void SemanticAnnotator::annotateLiteral(LiteralNode* node) {
    const std::string text = toLower(node->value);
    if (text == "true" || text == "false") {
        node->dataType = DataType::Boolean;
        return;
    }

    if (!text.empty() && text.front() == '\'' && text.back() == '\'' && text.size() >= 3) {
        node->dataType = DataType::Char;
        return;
    }

    bool hasRealMarker = false;
    for (char ch : text) {
        if (ch == '.' || ch == 'e') {
            hasRealMarker = true;
            break;
        }
    }
    if (hasRealMarker) {
        node->dataType = DataType::Real;
        return;
    }

    node->dataType = DataType::Integer;
}

void SemanticAnnotator::annotateArrayAccess(ArrayAccessNode* node) {
    if (node->children.size() < 2) {
        node->dataType = DataType::Unknown;
        return;
    }

    ASTNode* base = node->children[0];
    ASTNode* index = node->children[1];
    annotateNode(base);
    annotateValueNode(index);

    if (base->symbolEntry == nullptr && base->nodeType == NodeType::Identifier) {
        node->dataType = DataType::Unknown;
        return;
    }

    if (!isIntegerType(inferType(index))) {
        reportTypeMismatch(index, DataType::Integer, inferType(index), "array index");
    }

    node->symbolEntry = base->symbolEntry;
    const SymbolEntry* baseEntry = node->symbolEntry;
    if (baseEntry != nullptr && !baseEntry->isArray) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Subscripted value is not an array");
    } else if (baseEntry != nullptr && baseEntry->isArray) {
        int idxConst = 0;
        if (tryEvalIntConst(index, idxConst)) {
            const int depth = arrayAccessDepth(node);
            if (depth >= 0 && static_cast<std::size_t>(depth) < baseEntry->arrayBounds.size()) {
                const ArrayBound& bound = baseEntry->arrayBounds[static_cast<std::size_t>(depth)];
                if (idxConst < bound.lower || idxConst > bound.upper) {
                    errorHandler_.report(
                        index->pos.line,
                        index->pos.col,
                        "Array index out of bounds: " + std::to_string(idxConst) +
                            " not in [" + std::to_string(bound.lower) + ", " + std::to_string(bound.upper) + "]"
                    );
                }
            }
        }
    }

    node->dataType = inferType(base);
}

void SemanticAnnotator::annotateArrayType(ArrayTypeNode* node) {
    if (node->children.size() < 3) {
        node->dataType = DataType::Unknown;
        return;
    }

    annotateNode(node->children[0]);
    annotateNode(node->children[1]);
    node->dataType = inferTypeFromTypeNode(node->children[2]);
}

std::vector<ParamInfo> SemanticAnnotator::collectParams(ASTNode* paramList) const {
    std::vector<ParamInfo> result;
    if (paramList == nullptr) {
        return result;
    }

    auto collectFromParamDecl = [&](ParamDeclNode* paramDecl) {
        if (paramDecl == nullptr || paramDecl->children.size() < 2) {
            return;
        }
        ASTNode* idList = paramDecl->children[0];
        ASTNode* typeNode = paramDecl->children[1];
        const DataType paramType = inferTypeFromTypeNode(typeNode);

        auto appendOne = [&](IdentifierNode* id) {
            if (id == nullptr) {
                return;
            }
            ParamInfo info;
            info.name = id->identifier;
            info.type = paramType;
            info.isVarParam = paramDecl->isVar;
            result.push_back(info);
        };

        if (auto* list = dynamic_cast<ListNode*>(idList)) {
            for (ASTNode* item : list->children) {
                appendOne(dynamic_cast<IdentifierNode*>(item));
            }
            return;
        }
        appendOne(dynamic_cast<IdentifierNode*>(idList));
    };

    if (auto* single = dynamic_cast<ParamDeclNode*>(paramList)) {
        collectFromParamDecl(single);
        return result;
    }

    if (auto* list = dynamic_cast<ListNode*>(paramList)) {
        for (ASTNode* child : list->children) {
            collectFromParamDecl(dynamic_cast<ParamDeclNode*>(child));
        }
    }

    return result;
}

void SemanticAnnotator::checkCallArguments(ProcCallNode* node, const SymbolEntry* callee) {
    if (node == nullptr || callee == nullptr) {
        return;
    }

    const std::string calleeName = toLower(callee->name);
    if (calleeName == "read") {
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            ASTNode* arg = node->children[i];
            if (!isLValue(arg)) {
                errorHandler_.report(
                    arg->pos.line,
                    arg->pos.col,
                    "read expects assignable variable for parameter " + std::to_string(i + 1)
                );
            }
        }
        return;
    }

    if (calleeName == "write") {
        return;
    }

    const std::size_t expected = callee->params.size();
    const std::size_t actual = node->children.size();
    if (expected != actual) {
        errorHandler_.report(
            node->pos.line,
            node->pos.col,
            "Argument count mismatch in call to " + node->name +
                ": expected " + std::to_string(expected) +
                ", got " + std::to_string(actual)
        );
    }

    const std::size_t n = expected < actual ? expected : actual;
    for (std::size_t i = 0; i < n; ++i) {
        ASTNode* arg = node->children[i];
        const ParamInfo& param = callee->params[i];
        const DataType actualType = inferType(arg);

        if (!isAssignable(param.type, actualType)) {
            errorHandler_.report(
                arg->pos.line,
                arg->pos.col,
                "Argument type mismatch for parameter " + std::to_string(i + 1) +
                    " in call to " + node->name
            );
        }

        if (param.isVarParam && !isLValue(arg)) {
            errorHandler_.report(
                arg->pos.line,
                arg->pos.col,
                "var parameter requires assignable argument for parameter " +
                    std::to_string(i + 1) + " in call to " + node->name
            );
        }
    }
}

std::vector<ArrayBound> SemanticAnnotator::collectArrayBounds(ASTNode* typeNode) const {
    std::vector<ArrayBound> bounds;
    ASTNode* cur = typeNode;
    while (cur != nullptr && cur->nodeType == NodeType::ArrayType) {
        if (cur->children.size() < 3) {
            break;
        }
        int lower = 0;
        int upper = -1;
        if (tryEvalIntConst(cur->children[0], lower) && tryEvalIntConst(cur->children[1], upper)) {
            bounds.push_back({lower, upper});
        }
        cur = cur->children[2];
    }
    return bounds;
}

bool SemanticAnnotator::tryEvalIntConst(ASTNode* node, int& out) const {
    if (node == nullptr) {
        return false;
    }
    if (auto* lit = dynamic_cast<LiteralNode*>(node)) {
        const std::string s = toLower(lit->value);
        bool allDigits = !s.empty();
        for (char ch : s) {
            if (!std::isdigit(static_cast<unsigned char>(ch))) {
                allDigits = false;
                break;
            }
        }
        if (!allDigits) {
            return false;
        }
        out = std::stoi(s);
        return true;
    }
    if (auto* un = dynamic_cast<UnaryExprNode*>(node)) {
        if (un->children.empty()) {
            return false;
        }
        int v = 0;
        if (!tryEvalIntConst(un->children[0], v)) {
            return false;
        }
        const std::string op = toLower(un->op);
        if (op == "-" || op == "uminus") {
            out = -v;
            return true;
        }
        if (op == "+") {
            out = v;
            return true;
        }
    }
    if (auto* bin = dynamic_cast<BinaryExprNode*>(node)) {
        if (bin->children.size() < 2) {
            return false;
        }
        int lv = 0;
        int rv = 0;
        if (!tryEvalIntConst(bin->children[0], lv) || !tryEvalIntConst(bin->children[1], rv)) {
            return false;
        }
        const std::string op = toLower(bin->op);
        if (op == "+") {
            out = lv + rv;
            return true;
        }
        if (op == "-") {
            out = lv - rv;
            return true;
        }
        if (op == "*") {
            out = lv * rv;
            return true;
        }
        if (op == "div") {
            if (rv == 0) {
                return false;
            }
            out = lv / rv;
            return true;
        }
        if (op == "mod") {
            if (rv == 0) {
                return false;
            }
            out = lv % rv;
            return true;
        }
    }
    return false;
}

int SemanticAnnotator::arrayAccessDepth(const ASTNode* node) const {
    if (node == nullptr || node->nodeType != NodeType::ArrayAccess || node->children.empty()) {
        return 0;
    }
    const ASTNode* base = node->children[0];
    if (base != nullptr && base->nodeType == NodeType::ArrayAccess) {
        return arrayAccessDepth(base) + 1;
    }
    return 0;
}
/* 
    请不要删我的注释
    在当前函数名栈内，也就是递归调用的函数中，检查左值是否与当前函数同名
    如果是，则认为这是在为函数结果赋值
*/
bool SemanticAnnotator::isFunctionResultAssignment(ASTNode* lhs) const {
    if (functionContextStack_.empty()) {
        return false;
    }
    auto* id = dynamic_cast<IdentifierNode*>(lhs);
    if (id == nullptr) {
        return false;
    }
    return toLower(id->identifier) == functionContextStack_.back();
}

DataType SemanticAnnotator::inferType(ASTNode* node) const {
    if (node == nullptr) {
        return DataType::Unknown;
    }
    return node->dataType;
}

DataType SemanticAnnotator::inferTypeFromTypeNode(ASTNode* typeNode) const {
    if (typeNode == nullptr) {
        return DataType::Unknown;
    }

    if (typeNode->dataType != DataType::Unknown) {
        return typeNode->dataType;
    }

    if (auto* id = dynamic_cast<IdentifierNode*>(typeNode)) {
        const std::string typeName = toLower(id->identifier);
        if (typeName == "integer") {
            return DataType::Integer;
        }
        if (typeName == "real") {
            return DataType::Real;
        }
        if (typeName == "boolean") {
            return DataType::Boolean;
        }
        if (typeName == "char") {
            return DataType::Char;
        }
    }

    if (typeNode->nodeType == NodeType::ArrayType && typeNode->children.size() >= 3) {
        return inferTypeFromTypeNode(typeNode->children[2]);
    }

    return DataType::Unknown;
}

bool SemanticAnnotator::isAssignable(DataType lhs, DataType rhs) const {
    if (lhs == DataType::Unknown || rhs == DataType::Unknown) {
        return false;
    }
    // Allow widening conversion in assignments/calls: integer -> real.
    if (lhs == DataType::Real && rhs == DataType::Integer) {
        return true;
    }
    return lhs == rhs;
}

bool SemanticAnnotator::isBooleanType(DataType t) const {
    return t == DataType::Boolean;
}

bool SemanticAnnotator::isNumericType(DataType t) const {
    return t == DataType::Integer || t == DataType::Real;
}

bool SemanticAnnotator::isIntegerType(DataType t) const {
    return t == DataType::Integer;
}

bool SemanticAnnotator::isLValue(ASTNode* node) const {
    if (node == nullptr) {
        return false;
    }
    if (auto* id = dynamic_cast<IdentifierNode*>(node)) {
        return id->isLValue;
    }
    return node->nodeType == NodeType::ArrayAccess;
}

void SemanticAnnotator::reportTypeMismatch(ASTNode* node, DataType expected, DataType actual, const std::string& where) {
    errorHandler_.report(node->pos.line, node->pos.col,
        "Type mismatch in " + where + ": expected " + dataTypeToString(expected) +
        ", got " + dataTypeToString(actual));
}
