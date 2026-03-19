#include "ast.h"

ASTNode::ASTNode(NodeType type, SourcePos p) : nodeType(type), pos(p) {}

ProgramNode::ProgramNode(const std::string& programName, SourcePos p)
    : ASTNode(NodeType::Program, p), name(programName) {}

void ProgramNode::accept(ASTVisitor& visitor) {
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

BinaryExprNode::BinaryExprNode(const std::string& binaryOp, ASTNode* lhs, ASTNode* rhs, SourcePos p)
    : ASTNode(NodeType::BinaryExpr, p), op(binaryOp) {
    children.push_back(lhs);
    children.push_back(rhs);
}

void BinaryExprNode::accept(ASTVisitor& visitor) {
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

ArrayAccessNode::ArrayAccessNode(ASTNode* base, ASTNode* index, int arrayLowerBound, SourcePos p)
    : ASTNode(NodeType::ArrayAccess, p), lowerBound(arrayLowerBound) {
    children.push_back(base);
    children.push_back(index);
}

void ArrayAccessNode::accept(ASTVisitor& visitor) {
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

ProgramNode* ASTBuilder::makeProgram(const std::string& name, SourcePos pos) {
    return create<ProgramNode>(name, pos);
}

IdentifierNode* ASTBuilder::makeIdentifier(const std::string& identifier, SourcePos pos) {
    return create<IdentifierNode>(identifier, pos);
}

LiteralNode* ASTBuilder::makeLiteral(const std::string& value, SourcePos pos) {
    return create<LiteralNode>(value, pos);
}

BinaryExprNode* ASTBuilder::makeBinaryExpr(const std::string& op, ASTNode* lhs, ASTNode* rhs, SourcePos pos) {
    return create<BinaryExprNode>(op, lhs, rhs, pos);
}

AssignStmtNode* ASTBuilder::makeAssignStmt(ASTNode* lhs, ASTNode* rhs, SourcePos pos) {
    return create<AssignStmtNode>(lhs, rhs, pos);
}

IfStmtNode* ASTBuilder::makeIfStmt(ASTNode* cond, ASTNode* thenBranch, ASTNode* elseBranch, SourcePos pos) {
    return create<IfStmtNode>(cond, thenBranch, elseBranch, pos);
}

ArrayAccessNode* ASTBuilder::makeArrayAccess(ASTNode* base, ASTNode* index, int lowerBound, SourcePos pos) {
    return create<ArrayAccessNode>(base, index, lowerBound, pos);
}

ProcCallNode* ASTBuilder::makeProcCall(const std::string& name, const std::vector<ASTNode*>& args, SourcePos pos) {
    return create<ProcCallNode>(name, args, pos);
}
