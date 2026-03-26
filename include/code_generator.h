#ifndef PASCC_CODE_GENERATOR_H
#define PASCC_CODE_GENERATOR_H

#include <string>

#include "ast.h"
#include "symbol_table.h" // 需要访问符号表信息
#include "codegen_utils.h" // 代码生成辅助工具

// 代码生成器：基于Visitor模式遍历AST，生成等价C代码
class CodeGenerator : public ASTVisitor {
public:
    // 入口：生成C代码字符串
    std::string generate(ProgramNode* root);

    // Visitor接口：每种AST节点类型
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

    // 可选：重置生成器状态
    void reset();

    const std::string& getCurrentExpr() const { return currentExpr_; } // 添加访问器方法

protected:
    // 当前表达式/语句生成结果
    std::string currentExpr_; // 修改为 protected 以允许访问
    // 主体代码缓冲
    std::string body_;
    std::string emitNode(ASTNode* node); // 声明 emitNode 方法
};

#endif  // PASCC_CODE_GENERATOR_H
