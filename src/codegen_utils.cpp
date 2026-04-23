#include "codegen_utils.h"
#include "ast.h"
#include "symbol_table.h"
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

namespace {

std::string indentBlock(const std::string& text, int spaces) {
    if (text.empty() || spaces <= 0) {
        return text;
    }

    const std::string indent(static_cast<std::size_t>(spaces), ' ');
    std::ostringstream oss;
    std::size_t lineStart = 0;
    while (lineStart < text.size()) {
        std::size_t lineEnd = text.find('\n', lineStart);
        std::string line = text.substr(lineStart, lineEnd == std::string::npos ? std::string::npos : lineEnd - lineStart);
        if (!line.empty()) {
            oss << indent << line;
        }
        if (lineEnd == std::string::npos) {
            break;
        }
        oss << '\n';
        lineStart = lineEnd + 1;
    }
    return oss.str();
}

}  // namespace

static std::string pascalCharLiteralToCString(const std::string& text) {
    if (text.size() < 2 || text.front() != '\'' || text.back() != '\'') {
        return "\"\"";
    }
    std::string inner = text.substr(1, text.size() - 2);
    std::string escaped;
    escaped.reserve(inner.size() + 4);
    for (char ch : inner) {
        if (ch == '\\' || ch == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return "\"" + escaped + "\"";
}

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
    oss << "int main(void) {\n";
    if (!mainBody.empty()) {
        oss << indentBlock(mainBody, 4) << "\n";
    }
    oss << "    return 0;\n}\n";
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
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                std::string ctype = mapType(id->dataType);
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
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                std::string ptype = mapType(id->dataType);
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
        case DataType::Real:    return "float"; // Pascal real -> float
        case DataType::Boolean: return "int";
        case DataType::Char:    return "char";
        default:                return "int";
    }
}

// 获取C格式符
static std::string getFormat(DataType t) {
    switch (t) {
        case DataType::Integer: return "%d";
        case DataType::Real:    return "%f"; // Pascal real -> float, use %f
        case DataType::Boolean: return "%d";
        case DataType::Char:    return "%c";
        default:                return "%d";
    }
}

// 变量声明（支持数组、多变量、record类型）
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
    std::ostringstream oss;
    for (size_t i = 0; i < idList->children.size(); ++i) {
        IdentifierNode* id = dynamic_cast<IdentifierNode*>(idList->children[i]);
        if (!id) continue;

        // Check if this is a record type (user-defined type)
        if (id->symbolEntry && id->symbolEntry->type == DataType::Record &&
            !id->symbolEntry->typeName.empty()) {
            // Use the user-defined type name
            oss << id->symbolEntry->typeName << " " << id->identifier;
        } else {
            // Use standard type mapping
            std::string idType = CodegenUtils::mapType(id->dataType);
            oss << idType << " " << id->identifier;
        }

        for (int d : dimensions) {
            oss << "[" << d << "]";
        }
        oss << ";\n";
    }
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
    bool useCStringConst = false;
    std::string cStringLiteral;
    if (id->symbolEntry != nullptr && id->symbolEntry->isStringLikeConst) {
        useCStringConst = true;
        ctype = "char*";
    } else if (auto* lit = dynamic_cast<LiteralNode*>(val)) {
        if (dtype == DataType::Char && lit->isStringLikeLiteral) {
            useCStringConst = true;
            ctype = "char*";
            cStringLiteral = pascalCharLiteralToCString(lit->value);
        }
    }
    std::ostringstream oss;
    oss << "const " << ctype << " " << id->identifier << " = ";
    // 支持负号表达式
    if (useCStringConst) {
        if (cStringLiteral.empty()) {
            if (id->symbolEntry != nullptr && id->symbolEntry->hasConstLiteral) {
                cStringLiteral = pascalCharLiteralToCString(id->symbolEntry->constLiteralText);
            } else {
                cStringLiteral = "\"\"";
            }
        }
        oss << cStringLiteral;
    } else if (LiteralNode* lit = dynamic_cast<LiteralNode*>(val)) {
        if (lit->value == "true") {
            oss << "1";
        } else if (lit->value == "false") {
            oss << "0";
        } else {
            oss << lit->value;
        }
    } else if (UnaryExprNode* unary = dynamic_cast<UnaryExprNode*>(val)) {
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
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                std::string ctype = CodegenUtils::mapType(id->dataType);
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
            for (size_t j = 0; ids && j < ids->children.size(); ++j) {
                IdentifierNode* id = dynamic_cast<IdentifierNode*>(ids->children[j]);
                if (!id) continue;
                std::string ptype = CodegenUtils::mapType(id->dataType);
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
        DataType t = DataType::Integer;
        // 优先用 symbolEntry->type，保证与C声明一致
        if (arg != nullptr) {
            if (arg->symbolEntry && arg->symbolEntry->type != DataType::Unknown) {
                t = arg->symbolEntry->type;
            } else if (arg->dataType != DataType::Unknown) {
                t = arg->dataType;
            } else if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
                t = id->symbolEntry ? id->symbolEntry->type : DataType::Integer;
            }
        }
        oss << getFormat(t);

        if (arg) {
            CodeGenerator cg;
            std::string expr = cg.emitNode(arg);
            if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
                if (id->isFunctionResultTarget) {
                    args.push_back("&_retval");
                    continue;
                }
            }
            // var parameter identifier is emitted as (*x); scanf needs x in that case.
            if (expr.size() >= 4 && expr.rfind("(*", 0) == 0 && expr.back() == ')') {
                args.push_back(expr.substr(2, expr.size() - 3));
            } else {
                args.push_back("&" + expr);
            }
        } else {
            // Keep generated C compilable even for malformed AST.
            args.push_back("&0");
        }
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
        std::string fmt;
        bool stringLikeConst = false;
        if (arg != nullptr) {
            // Prefer semantic expression type first. Fallback to symbol type.
            if (arg->dataType != DataType::Unknown) {
                t = arg->dataType;
            } else if (arg->symbolEntry && arg->symbolEntry->type != DataType::Unknown) {
                t = arg->symbolEntry->type;
            } else if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
                t = id->symbolEntry ? id->symbolEntry->type : DataType::Integer;
            }

            if (IdentifierNode* id = dynamic_cast<IdentifierNode*>(arg)) {
                if (id->symbolEntry != nullptr &&
                    id->symbolEntry->isConstantLike() &&
                    id->symbolEntry->isStringLikeConst) {
                    stringLikeConst = true;
                }
            } else if (LiteralNode* lit = dynamic_cast<LiteralNode*>(arg)) {
                if (lit->isStringLikeLiteral) {
                    stringLikeConst = true;
                }
            }
        }
        fmt = stringLikeConst ? "%s" : getFormat(t);
        oss << fmt;
        if (arg) {
            CodeGenerator cg;
            std::string expr = cg.emitNode(arg);
            if (LiteralNode* lit = dynamic_cast<LiteralNode*>(arg)) {
                if (lit->isStringLikeLiteral) {
                    expr = pascalCharLiteralToCString(lit->value);
                }
            }
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
