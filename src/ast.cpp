#include "ast.h"

ASTNode::ASTNode(NodeType type, SourcePos p) : nodeType(type), pos(p) {}

// programstruct -> program_head ';' program_body '.'
ProgramNode::ProgramNode(const std::string& programName, SourcePos p)
    : ASTNode(NodeType::Program, p), name(programName) {}

void ProgramNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// program_body / subprogram_body structural container.
BlockNode::BlockNode(SourcePos p)
    : ASTNode(NodeType::Block, p) {}

void BlockNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// var_declaration -> idlist ':' type
// children[0] = idList, children[1] = typeNode
VarDeclNode::VarDeclNode(ASTNode* idList, ASTNode* typeNode, SourcePos p)
    : ASTNode(NodeType::VarDecl, p) {
    children.push_back(idList);
    children.push_back(typeNode);
}

void VarDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// const_declaration -> id '=' const_value
// children[0] = identifier, children[1] = literal/unary expression
ConstDeclNode::ConstDeclNode(ASTNode* id, ASTNode* value, SourcePos p)
    : ASTNode(NodeType::ConstDecl, p) {
    children.push_back(id);
    children.push_back(value);
}

void ConstDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// subprogram_head -> procedure id formal_parameter
// children[0] = parameter list, children[1] = procedure body block
ProcDeclNode::ProcDeclNode(const std::string& procName, ASTNode* paramList, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::ProcDecl, p), name(procName) {
    children.push_back(paramList);
    children.push_back(body);
}

void ProcDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// subprogram_head -> function id formal_parameter ':' basic_type
// children[0] = parameter list, children[1] = function body block
FuncDeclNode::FuncDeclNode(const std::string& funcName, ASTNode* paramList, DataType returnType, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::FuncDecl, p), name(funcName), retType(returnType) {
    children.push_back(paramList);
    children.push_back(body);
}

void FuncDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// statement -> variable assignop expression
// statement -> func_id assignop expression
// children[0] = lhs, children[1] = rhs
AssignStmtNode::AssignStmtNode(ASTNode* lhs, ASTNode* rhs, SourcePos p)
    : ASTNode(NodeType::AssignStmt, p) {
    children.push_back(lhs);
    children.push_back(rhs);
}

void AssignStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// statement -> if expression then statement else_part
// children[0] = cond, children[1] = then, children[2] = else (if present)
IfStmtNode::IfStmtNode(ASTNode* cond, ASTNode* thenBranch, ASTNode* elseBranch, SourcePos p)
    : ASTNode(NodeType::IfStmt, p) {
    children.push_back(cond);
    children.push_back(thenBranch);
    if (elseBranch != nullptr) {
        children.push_back(elseBranch);
    }
}

void IfStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// statement -> while expression do statement
// children: [condExpr, bodyStmt]
WhileStmtNode::WhileStmtNode(ASTNode* cond, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::WhileStmt, p) {
    children.push_back(cond);
    children.push_back(body);
}

void WhileStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// statement -> for id assignop expression to expression do statement
// statement -> for id assignop expression downto expression do statement
// children: [id, initExpr, endExpr, bodyStmt]
ForStmtNode::ForStmtNode(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, bool downto, SourcePos p)
    : ASTNode(NodeType::ForStmt, p), isDownto(downto) {
    children.push_back(id);
    children.push_back(init);
    children.push_back(end);
    children.push_back(body);
}

void ForStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

BreakStmtNode::BreakStmtNode(SourcePos p)
    : ASTNode(NodeType::BreakStmt, p) {}

void BreakStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// compound_statement -> begin statement_list end
// children keep source order of statements
CompoundStmtNode::CompoundStmtNode(const std::vector<ASTNode*>& stmts, SourcePos p)
    : ASTNode(NodeType::CompoundStmt, p) {
    for (ASTNode* stmt : stmts) {
        children.push_back(stmt);
    }
}

void CompoundStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// procedure_call -> id | id '(' expression_list ')'
// read/write are normalized as ProcCallNode in parser stage.
ProcCallNode::ProcCallNode(const std::string& procName, const std::vector<ASTNode*>& args, SourcePos p)
    : ASTNode(NodeType::ProcCall, p), name(procName) {
    for (ASTNode* arg : args) {
        children.push_back(arg);
    }
    isVarParam.assign(args.size(), false);
}

void ProcCallNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// expression/simple_expression/term binary composition.
// children[0] = lhs, children[1] = rhs
BinaryExprNode::BinaryExprNode(const std::string& binaryOp, ASTNode* lhs, ASTNode* rhs, SourcePos p)
    : ASTNode(NodeType::BinaryExpr, p), op(binaryOp) {
    children.push_back(lhs);
    children.push_back(rhs);
}

void BinaryExprNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// factor -> not factor | uminus factor
// children[0] = operand
UnaryExprNode::UnaryExprNode(const std::string& unaryOp, ASTNode* expr, SourcePos p)
    : ASTNode(NodeType::UnaryExpr, p), op(unaryOp) {
    children.push_back(expr);
}

void UnaryExprNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

IdentifierNode::IdentifierNode(const std::string& id, SourcePos p)
    : ASTNode(NodeType::Identifier, p), identifier(id) {}

void IdentifierNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// factor -> num (and normalized scalar literals)
LiteralNode::LiteralNode(const std::string& literalValue, SourcePos p)
    : ASTNode(NodeType::Literal, p), value(literalValue) {}

void LiteralNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// variable -> id id_varpart
// id_varpart -> '[' expression_list ']'
// lowerBound is reserved for semantic annotation stage.
ArrayAccessNode::ArrayAccessNode(ASTNode* base, ASTNode* index, int arrayLowerBound, SourcePos p)
    : ASTNode(NodeType::ArrayAccess, p), lowerBound(arrayLowerBound) {
    children.push_back(base);
    children.push_back(index);
}

void ArrayAccessNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// type -> array '[' period ']' of basic_type
// children: [lowerBoundExpr, upperBoundExpr, elemType]
// Multi-dimension arrays are represented via nested elemType.
ArrayTypeNode::ArrayTypeNode(ASTNode* lower, ASTNode* upper, ASTNode* elemType, SourcePos p)
    : ASTNode(NodeType::ArrayType, p) {
    children.push_back(lower);
    children.push_back(upper);
    children.push_back(elemType);
}

void ArrayTypeNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

// parameter -> var_parameter | value_parameter
// children[0] = idList, children[1] = typeNode
ParamDeclNode::ParamDeclNode(bool isVarParam, ASTNode* idList, ASTNode* typeNode, SourcePos p)
    : ASTNode(NodeType::ParamDecl, p), isVar(isVarParam) {
    children.push_back(idList);
    children.push_back(typeNode);
}

void ParamDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

ListNode::ListNode(ListKind listKind, SourcePos p)
    : ASTNode(NodeType::List, p), kind(listKind) {}

void ListNode::add(ASTNode* node) {
    // Null is ignored so optional grammar branches can safely call add().
    if (node != nullptr) {
        children.push_back(node);
    }
}

void ListNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}




// Builder methods are thin wrappers over arena allocation.
// They preserve constructor contracts and keep parser actions concise.
ProgramNode* ASTBuilder::makeProgram(const std::string& name, SourcePos pos) {
    return create<ProgramNode>(name, pos);
}

BlockNode* ASTBuilder::makeBlock(SourcePos pos) {
    return create<BlockNode>(pos);
}

VarDeclNode* ASTBuilder::makeVarDecl(ASTNode* idList, ASTNode* typeNode, SourcePos pos) {
    return create<VarDeclNode>(idList, typeNode, pos);
}

ConstDeclNode* ASTBuilder::makeConstDecl(ASTNode* id, ASTNode* value, SourcePos pos) {
    return create<ConstDeclNode>(id, value, pos);
}

ProcDeclNode* ASTBuilder::makeProcDecl(const std::string& name, ASTNode* paramList, ASTNode* body, SourcePos pos) {
    return create<ProcDeclNode>(name, paramList, body, pos);
}

FuncDeclNode* ASTBuilder::makeFuncDecl(const std::string& name, ASTNode* paramList, DataType retType, ASTNode* body, SourcePos pos) {
    return create<FuncDeclNode>(name, paramList, retType, body, pos);
}

AssignStmtNode* ASTBuilder::makeAssignStmt(ASTNode* lhs, ASTNode* rhs, SourcePos pos) {
    return create<AssignStmtNode>(lhs, rhs, pos);
}

IfStmtNode* ASTBuilder::makeIfStmt(ASTNode* cond, ASTNode* thenBranch, ASTNode* elseBranch, SourcePos pos) {
    return create<IfStmtNode>(cond, thenBranch, elseBranch, pos);
}

WhileStmtNode* ASTBuilder::makeWhileStmt(ASTNode* cond, ASTNode* body, SourcePos pos) {
    return create<WhileStmtNode>(cond, body, pos);
}

ForStmtNode* ASTBuilder::makeForStmt(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, bool isDownto, SourcePos pos) {
    return create<ForStmtNode>(id, init, end, body, isDownto, pos);
}

BreakStmtNode* ASTBuilder::makeBreakStmt(SourcePos pos) {
    return create<BreakStmtNode>(pos);
}

CompoundStmtNode* ASTBuilder::makeCompoundStmt(const std::vector<ASTNode*>& stmts, SourcePos pos) {
    return create<CompoundStmtNode>(stmts, pos);
}

ProcCallNode* ASTBuilder::makeProcCall(const std::string& name, const std::vector<ASTNode*>& args, SourcePos pos) {
    return create<ProcCallNode>(name, args, pos);
}

BinaryExprNode* ASTBuilder::makeBinaryExpr(const std::string& op, ASTNode* lhs, ASTNode* rhs, SourcePos pos) {
    return create<BinaryExprNode>(op, lhs, rhs, pos);
}

UnaryExprNode* ASTBuilder::makeUnaryExpr(const std::string& op, ASTNode* expr, SourcePos pos) {
    return create<UnaryExprNode>(op, expr, pos);
}


IdentifierNode* ASTBuilder::makeIdentifier(const std::string& identifier, SourcePos pos) {
    return create<IdentifierNode>(identifier, pos);
}

LiteralNode* ASTBuilder::makeLiteral(const std::string& value, SourcePos pos) {
    return create<LiteralNode>(value, pos);
}


ArrayAccessNode* ASTBuilder::makeArrayAccess(ASTNode* base, ASTNode* index, int lowerBound, SourcePos pos) {
    return create<ArrayAccessNode>(base, index, lowerBound, pos);
}

ArrayTypeNode* ASTBuilder::makeArrayType(ASTNode* lower, ASTNode* upper, ASTNode* elemType, SourcePos pos) {
    return create<ArrayTypeNode>(lower, upper, elemType, pos);
}

ParamDeclNode* ASTBuilder::makeParamDecl(bool isVar, ASTNode* idList, ASTNode* typeNode, SourcePos pos) {
    return create<ParamDeclNode>(isVar, idList, typeNode, pos);
}

ListNode* ASTBuilder::makeList(ListKind kind, SourcePos pos) {
    return create<ListNode>(kind, pos);
}

// Record 类型相关节点实现
TypeDeclNode::TypeDeclNode(const std::string& name, ASTNode* typeNode, SourcePos p)
    : ASTNode(NodeType::TypeDecl, p), name(name) {
    if (typeNode) children.push_back(typeNode);
}

void TypeDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

RecordTypeNode::RecordTypeNode(ASTNode* fieldList, SourcePos p)
    : ASTNode(NodeType::RecordType, p) {
    if (fieldList) children.push_back(fieldList);
}

void RecordTypeNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

FieldDeclNode::FieldDeclNode(ASTNode* idList, ASTNode* typeNode, SourcePos p)
    : ASTNode(NodeType::FieldDecl, p) {
    if (idList) children.push_back(idList);
    if (typeNode) children.push_back(typeNode);
}

void FieldDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

FieldAccessNode::FieldAccessNode(ASTNode* base, const std::string& fieldName, SourcePos p)
    : ASTNode(NodeType::FieldAccess, p), fieldName(fieldName) {
    if (base) children.push_back(base);
}

void FieldAccessNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

TypeDeclNode* ASTBuilder::makeTypeDecl(const std::string& name, ASTNode* typeNode, SourcePos pos) {
    return create<TypeDeclNode>(name, typeNode, pos);
}

RecordTypeNode* ASTBuilder::makeRecordType(ASTNode* fieldList, SourcePos pos) {
    return create<RecordTypeNode>(fieldList, pos);
}

FieldDeclNode* ASTBuilder::makeFieldDecl(ASTNode* idList, ASTNode* typeNode, SourcePos pos) {
    return create<FieldDeclNode>(idList, typeNode, pos);
}

FieldAccessNode* ASTBuilder::makeFieldAccess(ASTNode* base, const std::string& fieldName, SourcePos pos) {
    return create<FieldAccessNode>(base, fieldName, pos);
}


