#ifndef PASCC_CODE_GENERATOR_H
#define PASCC_CODE_GENERATOR_H

#include <string>

#include "ast.h"

class CodeGenerator : public ASTVisitor {
public:
    std::string generate(ProgramNode* root);

    void visit(ProgramNode* node) override;
    void visit(IdentifierNode* node) override;
    void visit(LiteralNode* node) override;
    void visit(BinaryExprNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(ArrayAccessNode* node) override;
    void visit(ProcCallNode* node) override;

private:
    std::string emitNode(ASTNode* node);

    std::string currentExpr_;
    std::string body_;
};

#endif  // PASCC_CODE_GENERATOR_H
