#include "codegen_utils.h"
#include "ast.h"
#include "symbol_table.h"
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

std::string CodegenUtils::wrapAsCProgram(const std::string& body) {
    return "#include <stdio.h>\n\nint main(void) {\n" + body + "\n    return 0;\n}\n";
}

// 类型映射
static std::string mapType(DataType t) {
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

    // 类型映射
    std::string ctype;
    int arrSize = -1;
    bool isArray = false;
    if (typeNode->nodeType == NodeType::ArrayType) {
        // 只支持一维数组
        ArrayTypeNode* arrType = dynamic_cast<ArrayTypeNode*>(typeNode);
        int lower = 0, upper = 0;
        if (LiteralNode* l = dynamic_cast<LiteralNode*>(arrType->children[0])) lower = std::stoi(l->value);
        if (LiteralNode* u = dynamic_cast<LiteralNode*>(arrType->children[1])) upper = std::stoi(u->value);
        arrSize = upper - lower + 1;
        ASTNode* elemType = arrType->children[2];
        ctype = mapType(elemType->dataType);
        isArray = true;
    } else {
        ctype = mapType(typeNode->dataType);
    }

    std::ostringstream oss;
    for (size_t i = 0; i < idList->children.size(); ++i) {
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(idList->children[i]);
        if (!id) continue;
        if (i > 0) oss << ", ";
        oss << ctype << " ";
        oss << id->identifier;
        if (isArray && arrSize > 0) {
            oss << "[" << arrSize << "]";
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
    LiteralNode* lit = dynamic_cast<LiteralNode*>(node->children[1]);
    if (!id || !lit) return "";
    // 统一用 const
    std::string ctype = mapType(id->dataType);
    std::ostringstream oss;
    oss << "const " << ctype << " " << id->identifier << " = " << lit->value << ";";
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
            std::string ctype = mapType(typeNode->dataType);
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
    std::string ctype = mapType(node->retType);
    oss << ctype << " " << node->name << "(";
    // 参数
    ListNode* paramList = dynamic_cast<ListNode*>(node->children[0]);
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
        if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
            t = id->symbolEntry ? id->symbolEntry->type : DataType::Integer;
        }
        oss << getFormat(t);
        args.push_back(arg->toString());
    }
    oss << "\"";
    for (const auto& a : args) oss << ", " << a;
    oss << ");";
    return oss.str();
}
