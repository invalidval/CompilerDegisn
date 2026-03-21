#include "ast.h"

ASTNode::ASTNode(NodeType type, SourcePos p) : nodeType(type), pos(p) {}

ProgramNode::ProgramNode(const std::string& programName, SourcePos p)
    : ASTNode(NodeType::Program, p), name(programName) {}

void ProgramNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

BlockNode::BlockNode(SourcePos p)
    : ASTNode(NodeType::Block, p) {}

void BlockNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

VarDeclNode::VarDeclNode(ASTNode* idList, ASTNode* typeNode, SourcePos p)
    : ASTNode(NodeType::VarDecl, p) {
    children.push_back(idList);
    children.push_back(typeNode);
}

void VarDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

ConstDeclNode::ConstDeclNode(ASTNode* id, ASTNode* value, SourcePos p)
    : ASTNode(NodeType::ConstDecl, p) {
    children.push_back(id);
    children.push_back(value);
}

void ConstDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

ProcDeclNode::ProcDeclNode(const std::string& procName, ASTNode* paramList, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::ProcDecl, p), name(procName) {
    children.push_back(paramList);
    children.push_back(body);
}

void ProcDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

FuncDeclNode::FuncDeclNode(const std::string& funcName, ASTNode* paramList, DataType returnType, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::FuncDecl, p), name(funcName), retType(returnType) {
    children.push_back(paramList);
    children.push_back(body);
}

void FuncDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

AssignStmtNode::AssignStmtNode(ASTNode* lhs, ASTNode* rhs, SourcePos p)
    : ASTNode(NodeType::AssignStmt, p) {
    children.push_back(lhs);
    children.push_back(rhs);
}

void AssignStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

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

ForStmtNode::ForStmtNode(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, SourcePos p)
    : ASTNode(NodeType::ForStmt, p) {
    children.push_back(id);
    children.push_back(init);
    children.push_back(end);
    children.push_back(body);
}

void ForStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

CompoundStmtNode::CompoundStmtNode(const std::vector<ASTNode*>& stmts, SourcePos p)
    : ASTNode(NodeType::CompoundStmt, p) {
    for (ASTNode* stmt : stmts) {
        children.push_back(stmt);
    }
}

void CompoundStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

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

BinaryExprNode::BinaryExprNode(const std::string& binaryOp, ASTNode* lhs, ASTNode* rhs, SourcePos p)
    : ASTNode(NodeType::BinaryExpr, p), op(binaryOp) {
    children.push_back(lhs);
    children.push_back(rhs);
}

void BinaryExprNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

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

LiteralNode::LiteralNode(const std::string& literalValue, SourcePos p)
    : ASTNode(NodeType::Literal, p), value(literalValue) {}

void LiteralNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

ArrayAccessNode::ArrayAccessNode(ASTNode* base, ASTNode* index, int arrayLowerBound, SourcePos p)
    : ASTNode(NodeType::ArrayAccess, p), lowerBound(arrayLowerBound) {
    children.push_back(base);
    children.push_back(index);
}

void ArrayAccessNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

ArrayTypeNode::ArrayTypeNode(ASTNode* lower, ASTNode* upper, ASTNode* elemType, SourcePos p)
    : ASTNode(NodeType::ArrayType, p) {
    children.push_back(lower);
    children.push_back(upper);
    children.push_back(elemType);
}

void ArrayTypeNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}

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
    if (node != nullptr) {
        children.push_back(node);
    }
}

void ListNode::accept(ASTVisitor& visitor) {
    visitor.visit(this);
}




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

ForStmtNode* ASTBuilder::makeForStmt(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, SourcePos pos) {
    return create<ForStmtNode>(id, init, end, body, pos);
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


