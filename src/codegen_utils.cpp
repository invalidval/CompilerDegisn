#include "codegen_utils.h"
#include "ast.h"
#include "symbol_table.h"
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

std::string CodegenUtils::wrapAsCProgram(const std::string& globals,
                                         const std::string& prototypes,
                                         const std::string& definitions,
                                         const std::string& mainBody) {
    std::ostringstream oss;
    // Plan B: 不引入 bool 头文件，布尔用整型表示
    oss << "#include <stdio.h>\n";
    if (!globals.empty()) oss << globals << "\n";
    if (!prototypes.empty()) oss << prototypes << "\n";
    if (!definitions.empty()) oss << definitions << "\n";
    oss << "int main(void) {\n" << mainBody << "\n    return 0;\n}\n";
    return oss.str();
}
// 过程原型声明
std::string CodegenUtils::emitProcPrototype(ProcDeclNode* node) {
    std::ostringstream oss;
    oss << "void " << node->name << "(";
    ListNode* paramList = dynamic_cast<ListNode*>(node->children[0]);
    bool first = true;
    if (paramList && !paramList->children.empty()) {
        for (size_t i = 0; i < paramList->children.size(); ++i) {
            ParamDeclNode* param = dynamic_cast<ParamDeclNode*>(paramList->children[i]);
            if (!param) continue;
            ListNode* ids = dynamic_cast<ListNode*>(param->children[0]);
            ASTNode* typeNode = param->children[1];
            std::string ctype = mapType(typeNode->dataType);
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                if (!first) oss << ", ";
                first = false;
                if (param->isVar) oss << ctype << " *" << id->identifier;
                else oss << ctype << " " << id->identifier;
            }
        }
    }
    oss << ");";
    return oss.str();
}

// 函数原型声明
std::string CodegenUtils::emitFuncPrototype(FuncDeclNode* node) {
    std::ostringstream oss;
    std::string ctype = mapType(node->retType);
    oss << ctype << " " << node->name << "(";
    ListNode* paramList = dynamic_cast<ListNode*>(node->children[0]);
    bool first = true;
    if (paramList && !paramList->children.empty()) {
        for (size_t i = 0; i < paramList->children.size(); ++i) {
            ParamDeclNode* param = dynamic_cast<ParamDeclNode*>(paramList->children[i]);
            if (!param) continue;
            ListNode* ids = dynamic_cast<ListNode*>(param->children[0]);
            ASTNode* typeNode = param->children[1];
            std::string ptype = mapType(typeNode->dataType);
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                if (!first) oss << ", ";
                first = false;
                if (param->isVar) oss << ptype << " *" << id->identifier;
                else oss << ptype << " " << id->identifier;
            }
        }
    }
    oss << ");";
    return oss.str();
}

// 类型映射
std::string CodegenUtils::mapType(DataType t) {
    switch (t) {
        case DataType::Integer: return "int";
        case DataType::Real:    return "double";
        case DataType::Boolean: return "int";
        case DataType::Char:    return "char";
        default:                return "int";
    }
}

// 获取C格式符
static std::string getFormat(DataType t) {
    switch (t) {
        case DataType::Integer: return "%d";
        case DataType::Real:    return "%lf";
        case DataType::Boolean: return "%d";
        case DataType::Char:    return "%c";
        default:                return "%d";
    }
}

// 变量声明（支持数组、多变量）
std::string CodegenUtils::emitVarDecl(VarDeclNode* node) {
    // children[0]: idList(ListNode), children[1]: typeNode
    if (!node || node->children.size() < 2) return "";
    ListNode* idList = dynamic_cast<ListNode*>(node->children[0]);
    ASTNode* typeNode = node->children[1];
    if (!idList) return "";

    // 多维数组支持：递归收集所有维度
    std::string ctype;
    std::vector<int> dimensions;
    ASTNode* elemType = typeNode;
    while (elemType->nodeType == NodeType::ArrayType) {
        ArrayTypeNode* arrType = dynamic_cast<ArrayTypeNode*>(elemType);
        int lower = 0, upper = 0;
        if (LiteralNode* l = dynamic_cast<LiteralNode*>(arrType->children[0])) lower = std::stoi(l->value);
        if (LiteralNode* u = dynamic_cast<LiteralNode*>(arrType->children[1])) upper = std::stoi(u->value);
        int arrSize = upper - lower + 1;
        dimensions.push_back(arrSize);
        elemType = arrType->children[2];
    }
    ctype = CodegenUtils::mapType(elemType->dataType);

    std::ostringstream oss;
    bool first = true;
    for (size_t i = 0; i < idList->children.size(); ++i) {
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(idList->children[i]);
        if (!id) continue;
        if (!first) oss << ", ";
        if (first) oss << ctype << " ";
        first = false;
        oss << id->identifier;
        // 输出所有维度
        for (int d : dimensions) {
            oss << "[" << d << "]";
        }
    }
    oss << ";";
    return oss.str();
}

// 常量声明
std::string CodegenUtils::emitConstDecl(ConstDeclNode* node) {
    // children[0]: identifier, children[1]: literal/unary
    if (!node || node->children.size() < 2) return "";
    IdentifierNode* id = dynamic_cast<IdentifierNode*>(node->children[0]);
    ASTNode* val = node->children[1];
    DataType dtype = DataType::Integer;
    if (val) dtype = val->dataType;
    std::string ctype = CodegenUtils::mapType(dtype);
    std::ostringstream oss;
    oss << "const " << ctype << " " << id->identifier << " = ";
    // 支持负号表达式
    if (auto* lit = dynamic_cast<LiteralNode*>(val)) {
        if (lit->value == "true") {
            oss << "1";
        } else if (lit->value == "false") {
            oss << "0";
        } else {
            oss << lit->value;
        }
    } else if (auto* unary = dynamic_cast<UnaryExprNode*>(val)) {
        oss << unary->op << dynamic_cast<LiteralNode*>(unary->children[0])->value;
    } else {
        oss << "0";
    }
    oss << ";";
    return oss.str();
}

// 过程声明
std::string CodegenUtils::emitProcDecl(ProcDeclNode* node, CodeGenerator& cg) {
    // children[0]: paramList(ListNode), children[1]: body(BlockNode)
    if (!node || node->children.size() < 2) return "";
    std::ostringstream oss;
    oss << "void " << node->name << "(";
    // 参数
    ListNode* paramList = dynamic_cast<ListNode*>(node->children[0]);
    if (paramList && !paramList->children.empty()) {
        for (size_t i = 0; i < paramList->children.size(); ++i) {
            ParamDeclNode* param = dynamic_cast<ParamDeclNode*>(paramList->children[i]);
            if (!param) continue;
            ListNode* ids = dynamic_cast<ListNode*>(param->children[0]);
            ASTNode* typeNode = param->children[1];
            std::string ctype = CodegenUtils::mapType(typeNode->dataType);
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                if (i > 0 || j > 0) oss << ", ";
                if (param->isVar) oss << ctype << " *" << id->identifier;
                else oss << ctype << " " << id->identifier;
            }
        }
    }
    oss << ") {\n";
    // 过程体
    BlockNode* body = dynamic_cast<BlockNode*>(node->children[1]);
    if (body) {
        cg.visit(body);
        oss << cg.getCurrentExpr(); // 使用访问器方法获取 currentExpr_ 的值
    }
    oss << "\n}\n";
    return oss.str();
}

// 函数声明
std::string CodegenUtils::emitFuncDecl(FuncDeclNode* node, CodeGenerator& cg) {
    // children[0]: paramList(ListNode), children[1]: body(BlockNode)
    if (!node || node->children.size() < 2) return "";
    std::ostringstream oss;
    std::string ctype = CodegenUtils::mapType(node->retType);
    oss << ctype << " " << node->name << "(";
    // 参数
    ListNode* paramList = dynamic_cast<ListNode*>(node->children[0]);
    if (paramList && !paramList->children.empty()) {
        for (size_t i = 0; i < paramList->children.size(); ++i) {
            ParamDeclNode* param = dynamic_cast<ParamDeclNode*>(paramList->children[i]);
            if (!param) continue;
            ListNode* ids = dynamic_cast<ListNode*>(param->children[0]);
            ASTNode* typeNode = param->children[1];
            std::string ptype = CodegenUtils::mapType(typeNode->dataType);
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                if (i > 0 || j > 0) oss << ", ";
                if (param->isVar) oss << ptype << " *" << id->identifier;
                else oss << ptype << " " << id->identifier;
            }
        }
    }
    oss << ") {\n";
    // _retval变量
    oss << "    " << ctype << " _retval;\n";
    // 函数体
    BlockNode* body = dynamic_cast<BlockNode*>(node->children[1]);
    if (body) {
        cg.visit(body);
        oss << cg.getCurrentExpr(); // 使用访问器方法获取 currentExpr_ 的值
    }
    oss << "\n    return _retval;\n";
    oss << "}\n";
    return oss.str();
}

// read语句
std::string CodegenUtils::emitReadStmt(ProcCallNode* node) {
    // node->children: 变量列表
    std::ostringstream oss;
    oss << "scanf(\"";
    std::vector<std::string> args;
    for (ASTNode* arg : node->children) {
        // 变量类型
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg);
        DataType t = id && id->symbolEntry ? id->symbolEntry->type : DataType::Integer;
        oss << getFormat(t);
        args.push_back("&" + (id ? id->identifier : "var"));
    }
    oss << "\"";
    for (const auto& a : args) oss << ", " << a;
    oss << ");";
    return oss.str();
}

// write语句
std::string CodegenUtils::emitWriteStmt(ProcCallNode* node) {
    // node->children: 表达式列表
    std::ostringstream oss;
    oss << "printf(\"";
    std::vector<std::string> args;
    for (ASTNode* arg : node->children) {
        DataType t = DataType::Integer;
        // 类型推断
        if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
            t = id->symbolEntry ? id->symbolEntry->type : DataType::Integer;
        }
        oss << getFormat(t);
        // 递归生成表达式代码
        if (arg) {
            // 使用CodeGenerator递归生成表达式代码
            CodeGenerator cg;
            std::string expr = cg.emitNode(arg);
            args.push_back(expr);
        } else {
            args.push_back("0");
        }
    }
    oss << "\"";
    for (const auto& a : args) oss << ", " << a;
    oss << ");";
    return oss.str();
}
