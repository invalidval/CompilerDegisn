#ifndef PASCC_CODEGEN_UTILS_H
#define PASCC_CODEGEN_UTILS_H

#include <string>
#include "code_generator.h" // 确保包含 CodeGenerator 的完整定义

class VarDeclNode;
class ConstDeclNode;
class ProcDeclNode;
class FuncDeclNode;
class ProcCallNode;

class CodeGenerator; // 前置声明

class CodegenUtils {
public:
    // Pascal-S类型到C类型映射
    static std::string mapType(DataType t);
    static std::string wrapAsCProgram(const std::string& globals,
                                      const std::string& prototypes,
                                      const std::string& definitions,
                                      const std::string& mainBody);

    // 变量声明
    static std::string emitVarDecl(VarDeclNode* node);
    // 常量声明
    static std::string emitConstDecl(ConstDeclNode* node);
    // 过程声明
    static std::string emitProcDecl(ProcDeclNode* node, CodeGenerator& cg);
    // 过程原型声明
    static std::string emitProcPrototype(ProcDeclNode* node);
    // 函数声明
    static std::string emitFuncDecl(FuncDeclNode* node, CodeGenerator& cg);
    // 函数原型声明
    static std::string emitFuncPrototype(FuncDeclNode* node);
    // read语句
    static std::string emitReadStmt(ProcCallNode* node);
    // write语句
    static std::string emitWriteStmt(ProcCallNode* node);
};

#endif  // PASCC_CODEGEN_UTILS_H
