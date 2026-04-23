#ifndef PASCC_SEMANTIC_ANNOTATOR_H
#define PASCC_SEMANTIC_ANNOTATOR_H

#include "ast.h"
#include "error_handler.h"
#include "symbol_table.h"

#include <vector>

class SemanticAnnotator {
public:
    SemanticAnnotator(SymbolTable& symbolTable, ErrorHandler& errorHandler);

    void annotate(ASTNode* root);

private:
    void annotateNode(ASTNode* node);
    void annotateProgram(ProgramNode* node);
    void annotateBlock(BlockNode* node);

    void annotateList(ListNode* node);

    void annotateConstDecl(ConstDeclNode* node);
    void annotateVarDecl(VarDeclNode* node);
    void annotateParamDecl(ParamDeclNode* node);
    void annotateProcDecl(ProcDeclNode* node);
    void annotateFuncDecl(FuncDeclNode* node);
    void annotateTypeDecl(TypeDeclNode* node);
    void annotateRecordType(RecordTypeNode* node);
    void annotateFieldDecl(FieldDeclNode* node);
    void annotateFieldAccess(FieldAccessNode* node);

    void annotateAssignStmt(AssignStmtNode* node);
    void annotateIfStmt(IfStmtNode* node);
    void annotateWhileStmt(WhileStmtNode* node);
    void annotateForStmt(ForStmtNode* node);
    void annotateBreakStmt(BreakStmtNode* node);
    void annotateCompoundStmt(CompoundStmtNode* node);
    void annotateProcCall(ProcCallNode* node);

    void annotateBinaryExpr(BinaryExprNode* node);
    void annotateUnaryExpr(UnaryExprNode* node);
    void annotateIdentifier(IdentifierNode* node);
    void annotateLiteral(LiteralNode* node);
    void annotateArrayAccess(ArrayAccessNode* node);
    void annotateArrayType(ArrayTypeNode* node);
    void annotateValueNode(ASTNode* node);
    bool isValueContext() const;

    std::vector<ParamInfo> collectParams(ASTNode* paramList) const;
    void checkCallArguments(ProcCallNode* node, const SymbolEntry* callee);
    std::vector<ArrayBound> collectArrayBounds(ASTNode* typeNode) const;
    bool tryEvalIntConst(ASTNode* node, int& out) const;
    int arrayAccessDepth(const ASTNode* node) const;
    bool isFunctionResultAssignment(ASTNode* lhs) const;

    DataType inferType(ASTNode* node) const;
    DataType inferTypeFromTypeNode(ASTNode* typeNode) const;

    bool isAssignable(DataType lhs, DataType rhs) const;
    bool isBooleanType(DataType t) const;
    bool isNumericType(DataType t) const;
    bool isIntegerType(DataType t) const;
    bool isLValue(ASTNode* node) const;

    void reportTypeMismatch(ASTNode* node, DataType expected, DataType actual, const std::string& where);

    SymbolTable& symbolTable_;
    ErrorHandler& errorHandler_;

    std::vector<std::string> functionContextStack_;
    int valueContextDepth_ = 0;
    int loopDepth_ = 0;
};

#endif  // PASCC_SEMANTIC_ANNOTATOR_H
