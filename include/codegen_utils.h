#ifndef PASCC_CODEGEN_UTILS_H
#define PASCC_CODEGEN_UTILS_H

#include <string>
class VarDeclNode;
class ConstDeclNode;
class ProcDeclNode;
class FuncDeclNode;
class ProcCallNode;

class CodeGenerator; // 前置声明

class CodegenUtils {
public:
    static std::string wrapAsCProgram(const std::string& body);

    // 变量声明
    static std::string emitVarDecl(VarDeclNode* node);
    // 常量声明
    static std::string emitConstDecl(ConstDeclNode* node);
    // 过程声明
    static std::string emitProcDecl(ProcDeclNode* node, CodeGenerator& cg);
    // 函数声明
    static std::string emitFuncDecl(FuncDeclNode* node, CodeGenerator& cg);
    // read语句
    static std::string emitReadStmt(ProcCallNode* node);
    // write语句
    static std::string emitWriteStmt(ProcCallNode* node);
};

#endif  // PASCC_CODEGEN_UTILS_H
