#ifndef PASCC_CODE_GENERATOR_H
#define PASCC_CODE_GENERATOR_H

#include <string>

#include "ast.h"

class CodeGenerator : public ASTVisitor {
public:
    std::string generate(ProgramNode* root);

    void visit(ProgramNode* node) override;
    void visit(BlockNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(ProcDeclNode* node) override;
    void visit(FuncDeclNode* node) override;
    void visit(IdentifierNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(BinaryExprNode* node) override;
    void visit(UnaryExprNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(ForStmtNode* node) override;
    void visit(CompoundStmtNode* node) override;
    void visit(ArrayAccessNode* node) override;
    void visit(ArrayTypeNode* node) override;
    void visit(ParamDeclNode* node) override;
    void visit(ListNode* node) override;
    void visit(ProcCallNode* node) override;

private:
    std::string emitNode(ASTNode* node);

    std::string currentExpr_;
    std::string body_;
};

#endif  // PASCC_CODE_GENERATOR_H
