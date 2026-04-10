#include "code_generator.h"
#include "codegen_utils.h"
#include "symbol_table.h" // 新增：引入符号表类型定义

#include <sstream>
#include <cctype>

namespace {
std::string toLowerCopy(const std::string& text) {
    std::string out = text;
    for (char& ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

std::string indentText(const std::string& text, int spaces) {
    if (text.empty() || spaces <= 0) {
        return text;
    }

    const std::string indent(static_cast<std::size_t>(spaces), ' ');
    std::ostringstream oss;
    std::size_t lineStart = 0;
    while (lineStart < text.size()) {
        std::size_t lineEnd = text.find('\n', lineStart);
        std::string line = text.substr(lineStart, lineEnd == std::string::npos ? std::string::npos : lineEnd - lineStart);
        if (!line.empty()) {
            oss << indent << line;
        }
        if (lineEnd == std::string::npos) {
            break;
        }
        oss << '\n';
        lineStart = lineEnd + 1;
    }
    return oss.str();
}

int arrayAccessDepthForCodegen(const ASTNode* node) {
    if (node == nullptr || node->nodeType != NodeType::ArrayAccess || node->children.empty()) {
        return 0;
    }
    const ASTNode* base = node->children[0];
    if (base != nullptr && base->nodeType == NodeType::ArrayAccess) {
        return arrayAccessDepthForCodegen(base) + 1;
    }
    return 0;
}

bool needsTrailingSemicolon(const ASTNode* node) {
    if (node == nullptr) {
        return false;
    }
    return node->nodeType == NodeType::ProcCall ||
           node->nodeType == NodeType::BreakStmt;
}
}

std::string CodeGenerator::generate(ProgramNode* root) {
    reset();
    if (!root) return "";
    // 递归遍历 AST，分区收集
    // 约定：
    // BlockNode: children = [consts, vars, subprograms, compound]
    // subprograms: ListNode，每个元素为 ProcDeclNode/FuncDeclNode
    // compound: CompoundStmtNode
    BlockNode* block = nullptr;
    for (ASTNode* child : root->children) {
        block = dynamic_cast<BlockNode*>(child);
        if (block) break;
    }
    if (!block) return "/* Invalid AST: no program body */\n";

    // 1. 全局常量/变量声明
    if (block->children.size() > 0) {
        ListNode* consts = dynamic_cast<ListNode*>(block->children[0]);
        if (consts) {
            for (ASTNode* decl : consts->children) {
                ConstDeclNode* cdecl = dynamic_cast<ConstDeclNode*>(decl);
                if (cdecl) globalDecls_ += CodegenUtils::emitConstDecl(cdecl) + "\n";
            }
        }
    }
    if (block->children.size() > 1) {
        ListNode* vars = dynamic_cast<ListNode*>(block->children[1]);
        if (vars) {
            for (ASTNode* decl : vars->children) {
                VarDeclNode* vdecl = dynamic_cast<VarDeclNode*>(decl);
                if (vdecl) globalDecls_ += CodegenUtils::emitVarDecl(vdecl) + "\n";
            }
        }
    }

    // 2. 子程序原型和定义
    if (block->children.size() > 2) {
        ListNode* subs = dynamic_cast<ListNode*>(block->children[2]);
        if (subs) {
            for (ASTNode* sub : subs->children) {
                if (auto* proc = dynamic_cast<ProcDeclNode*>(sub)) {
                    prototypes_ += CodegenUtils::emitProcPrototype(proc) + "\n";
                    definitions_ += CodegenUtils::emitProcDecl(proc, *this) + "\n";
                } else if (auto* func = dynamic_cast<FuncDeclNode*>(sub)) {
                    prototypes_ += CodegenUtils::emitFuncPrototype(func) + "\n";
                    definitions_ += CodegenUtils::emitFuncDecl(func, *this) + "\n";
                }
            }
        }
    }

    // 3. 主程序体
    if (block->children.size() > 3) {
        CompoundStmtNode* mainStmt = dynamic_cast<CompoundStmtNode*>(block->children[3]);
        if (mainStmt) {
            visit(mainStmt);
            mainBody_ = currentExpr_;
        }
    }

    // 4. 组装
    return CodegenUtils::wrapAsCProgram(globalDecls_, prototypes_, definitions_, mainBody_);
}

void CodeGenerator::reset() {
    globalDecls_.clear();
    prototypes_.clear();
    definitions_.clear();
    mainBody_.clear();
    currentExpr_.clear();
}

void CodeGenerator::visit(ProgramNode* node) {
    // 不做任何事，主流程在 generate 里
}

void CodeGenerator::visit(BlockNode* node) {
    std::ostringstream oss;
    // BlockNode: children = [consts, vars, subprograms, compound]
    if (node->children.size() > 0) {
        ListNode* consts = dynamic_cast<ListNode*>(node->children[0]);
        if (consts) {
            for (ASTNode* decl : consts->children) {
                ConstDeclNode* cdecl = dynamic_cast<ConstDeclNode*>(decl);
                if (!cdecl) {
                    continue;
                }
                std::string line = CodegenUtils::emitConstDecl(cdecl);
                if (!line.empty()) {
                    oss << "    " << line << "\n";
                }
            }
        }
    }

    if (node->children.size() > 1) {
        ListNode* vars = dynamic_cast<ListNode*>(node->children[1]);
        if (vars) {
            for (ASTNode* decl : vars->children) {
                VarDeclNode* vdecl = dynamic_cast<VarDeclNode*>(decl);
                if (!vdecl) {
                    continue;
                }
                std::string line = CodegenUtils::emitVarDecl(vdecl);
                if (!line.empty()) {
                    oss << indentText(line, 4) << "\n";
                }
            }
        }
    }

    ASTNode* bodyNode = nullptr;
    if (node->children.size() > 3) {
        bodyNode = node->children[3];
    } else if (node->children.size() > 2) {
        // subprogram_body: [consts, vars, compound]
        bodyNode = node->children[2];
    }

    if (bodyNode) {
        std::string body = emitNode(bodyNode);
        if (!body.empty()) {
            oss << indentText(body, 4);
            if (body.back() != '\n') {
                oss << "\n";
            }
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(VarDeclNode* node) {
    // 变量声明只在 generate 里处理，这里不输出
    currentExpr_.clear();
}

void CodeGenerator::visit(ConstDeclNode* node) {
    // 常量声明只在 generate 里处理，这里不输出
    currentExpr_.clear();
}

void CodeGenerator::visit(ProcDeclNode* node) {
    // 过程定义只在 generate 里处理，这里不输出
    currentExpr_.clear();
}

void CodeGenerator::visit(FuncDeclNode* node) {
    // 函数定义只在 generate 里处理，这里不输出
    currentExpr_.clear();
}

void CodeGenerator::visit(IdentifierNode* node) {
    const std::string lowered = toLowerCopy(node->identifier);
    if (lowered == "true") {
        currentExpr_ = "1";
        return;
    }
    if (lowered == "false") {
        currentExpr_ = "0";
        return;
    }

    if (node->symbolEntry != nullptr &&
        node->symbolEntry->kind == SymbolKind::Parameter &&
        node->symbolEntry->isVarParam) {
        // var parameter is translated to C pointer parameter.
        // In expression/assignment context it should be dereferenced.
        currentExpr_ = "(*" + node->identifier + ")";
        return;
    }

    if (node->symbolEntry != nullptr &&
        node->symbolEntry->kind == SymbolKind::Function) {
        // Pascal allows zero-arg function designator in expressions, e.g. write(f).
        currentExpr_ = node->identifier + "()";
        return;
    }

    currentExpr_ = node->identifier;
}

void CodeGenerator::visit(LiteralNode* node) {
    const std::string lowered = toLowerCopy(node->value);
    if (lowered == "true") {
        currentExpr_ = "1";
        return;
    }
    if (lowered == "false") {
        currentExpr_ = "0";
        return;
    }
    currentExpr_ = node->value;
}

void CodeGenerator::visit(BinaryExprNode* node) {
    std::string lhs = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string rhs = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    std::string op = node->op;
    if (op == "mod") op = "%";
    else if (op == "div") op = "/";
    else if (op == "=") op = "==";
    else if (op == "<>") op = "!=";
    else if (op == "and") op = "&&";
    else if (op == "or") op = "||";
    currentExpr_ = "(" + lhs + " " + op + " " + rhs + ")";
}

void CodeGenerator::visit(UnaryExprNode* node) {
    std::string operand = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    if (node->op == "not") {
        if (node->dataType == DataType::Integer) {
            currentExpr_ = "(~" + operand + ")";
        } else {
            currentExpr_ = "(!" + operand + ")";
        }
    } else {
        currentExpr_ = "(" + node->op + operand + ")";
    }
}

void CodeGenerator::visit(AssignStmtNode* node) {
    // 判断是否为函数返回值赋值
    IdentifierNode* idNode = dynamic_cast<IdentifierNode*>(node->children[0]);
    if (idNode && idNode->symbolEntry) {
        // 正确判断：符号种类为函数
        if (idNode->symbolEntry->kind == SymbolKind::Function) {
            std::string rhs = emitNode(node->children[1]);
            currentExpr_ = "_retval = " + rhs + ";";
            return;
        }
    }
    // 普通赋值
    std::string lhs = emitNode(node->children[0]);
    std::string rhs = emitNode(node->children[1]);
    currentExpr_ = lhs + " = " + rhs + ";";
}

void CodeGenerator::visit(IfStmtNode* node) {
    std::string cond = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string thenStmt = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    if (node->children.size() > 1 && needsTrailingSemicolon(node->children[1]) &&
        !thenStmt.empty() && thenStmt.back() != ';') {
        thenStmt.push_back(';');
    }

    std::ostringstream oss;
    oss << "if (" << cond << ") {\n";
    if (!thenStmt.empty()) {
        oss << indentText(thenStmt, 4);
        if (thenStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "}";

    if (node->children.size() > 2) {
        std::string elseStmt = emitNode(node->children[2]);
        if (needsTrailingSemicolon(node->children[2]) &&
            !elseStmt.empty() && elseStmt.back() != ';') {
            elseStmt.push_back(';');
        }
        oss << " else {\n";
        if (!elseStmt.empty()) {
            oss << indentText(elseStmt, 4);
            if (elseStmt.back() != '\n') {
                oss << "\n";
            }
        }
        oss << "}";
    }

    currentExpr_ = oss.str();
}

void CodeGenerator::visit(WhileStmtNode* node) {
    std::string cond = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string bodyStmt = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    if (node->children.size() > 1 && needsTrailingSemicolon(node->children[1]) &&
        !bodyStmt.empty() && bodyStmt.back() != ';') {
        bodyStmt.push_back(';');
    }

    std::ostringstream oss;
    oss << "while (" << cond << ") {\n";
    if (!bodyStmt.empty()) {
        oss << indentText(bodyStmt, 4);
        if (bodyStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "}";
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ForStmtNode* node) {
    std::string iter = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string init = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    std::string end = emitNode(node->children.size() > 2 ? node->children[2] : nullptr);
    std::string bodyStmt = emitNode(node->children.size() > 3 ? node->children[3] : nullptr);
    if (node->children.size() > 3 && needsTrailingSemicolon(node->children[3]) &&
        !bodyStmt.empty() && bodyStmt.back() != ';') {
        bodyStmt.push_back(';');
    }

    std::string cmp = node->isDownto ? ">=" : "<=";
    std::string step = node->isDownto ? "--" : "++";

    std::ostringstream oss;
    IdentifierNode* iterId = dynamic_cast<IdentifierNode*>(node->children.size() > 0 ? node->children[0] : nullptr);
    bool needLoopVarDecl = (iterId != nullptr && iterId->symbolEntry == nullptr);
    if (needLoopVarDecl) {
        oss << "for (int " << iter << " = " << init << "; " << iter << " " << cmp << " " << end;
    } else {
        oss << "for (" << iter << " = " << init << "; " << iter << " " << cmp << " " << end;
    }
    oss
        << "; " << iter << step << ") {\n";
    if (!bodyStmt.empty()) {
        oss << indentText(bodyStmt, 4);
        if (bodyStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "}";
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(BreakStmtNode* /*node*/) {
    currentExpr_ = "break";
}

void CodeGenerator::visit(CompoundStmtNode* node) {
    std::ostringstream oss;
    for (ASTNode* stmt : node->children) {
        std::string line = emitNode(stmt);
        if (needsTrailingSemicolon(stmt) && !line.empty() && line.back() != ';') {
            line.push_back(';');
        }
        if (!line.empty()) {
            oss << line;
            if (line.back() != '\n') {
                oss << "\n";
            }
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    std::string base = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string index = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);

    int lowerBound = node->lowerBound;
    if (node->symbolEntry != nullptr && node->symbolEntry->isArray) {
        const int depth = arrayAccessDepthForCodegen(node);
        if (depth >= 0 && static_cast<size_t>(depth) < node->symbolEntry->arrayBounds.size()) {
            lowerBound = node->symbolEntry->arrayBounds[static_cast<size_t>(depth)].lower;
        }
    }

    if (lowerBound == 0) {
        currentExpr_ = base + "[" + index + "]";
    } else {
        currentExpr_ = base + "[(" + index + ") - " + std::to_string(lowerBound) + "]";
    }
}

void CodeGenerator::visit(ArrayTypeNode* /*node*/) {
    currentExpr_.clear();
}

void CodeGenerator::visit(ParamDeclNode* /*node*/) {
    currentExpr_.clear();
}

void CodeGenerator::visit(ListNode* node) {
    std::ostringstream oss;
    for (ASTNode* item : node->children) {
        std::string piece = emitNode(item);
        if (!piece.empty()) {
            if (oss.tellp() > 0) {
                oss << ", ";
            }
            oss << piece;
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ProcCallNode* node) {
    if (node->builtinKind == BuiltinProcKind::Read) {
        std::string code = CodegenUtils::emitReadStmt(node);
        currentExpr_ = code;
        return;
    }
    if (node->builtinKind == BuiltinProcKind::Write) {
        std::string code = CodegenUtils::emitWriteStmt(node);
        currentExpr_ = code;
        return;
    }
    // 普通过程调用
    std::ostringstream oss;
    oss << node->name << "(";

    for (size_t i = 0; i < node->children.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }

        ASTNode* rawArgNode = node->children[i];
        std::string arg = emitNode(rawArgNode);
        if (i < node->isVarParam.size() && node->isVarParam[i]) {
            if (auto* id = dynamic_cast<IdentifierNode*>(rawArgNode)) {
                if (id->symbolEntry != nullptr &&
                    id->symbolEntry->kind == SymbolKind::Parameter &&
                    id->symbolEntry->isVarParam) {
                    oss << id->identifier;
                } else {
                    oss << "&" << arg;
                }
            } else {
                oss << "&" << arg;
            }
        } else {
            oss << arg;
        }
    }

    oss << ")";
    currentExpr_ = oss.str();
}

std::string CodeGenerator::emitNode(ASTNode* node) {
    if (node == nullptr) {
        return "";
    }

    node->accept(*this); // 调用对应节点的 visit 方法
    return currentExpr_; // 返回生成的代码
}
