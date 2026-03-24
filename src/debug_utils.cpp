#include "debug_utils.h"
#include "ast.h"
#include <iostream>

namespace {

const char* nodeTypeName(NodeType type) {
    return type == NodeType::Program ? "Program" :
           type == NodeType::Block ? "Block" :
           type == NodeType::VarDecl ? "VarDecl" :
           type == NodeType::ConstDecl ? "ConstDecl" :
           type == NodeType::ProcDecl ? "ProcDecl" :
           type == NodeType::FuncDecl ? "FuncDecl" :
           type == NodeType::AssignStmt ? "AssignStmt" :
           type == NodeType::IfStmt ? "IfStmt" :
           type == NodeType::ForStmt ? "ForStmt" :
           type == NodeType::CompoundStmt ? "CompoundStmt" :
           type == NodeType::ProcCall ? "ProcCall" :
           type == NodeType::BinaryExpr ? "BinaryExpr" :
           type == NodeType::UnaryExpr ? "UnaryExpr" :
           type == NodeType::Identifier ? "Identifier" :
           type == NodeType::Literal ? "Literal" :
           type == NodeType::ArrayAccess ? "ArrayAccess" :
           type == NodeType::ArrayType ? "ArrayType" :
           type == NodeType::ParamDecl ? "ParamDecl" :
           type == NodeType::List ? "List" : "UnknownNode";
}

const char* dataTypeName(DataType type) {
    return type == DataType::Integer ? "integer" :
           type == DataType::Real ? "real" :
           type == DataType::Boolean ? "boolean" :
           type == DataType::Char ? "char" :
           type == DataType::Procedure ? "procedure" :
           type == DataType::Function ? "function" : "unknown";
}

const char* symbolKindName(SymbolKind kind) {
    return kind == SymbolKind::Variable ? "variable" :
           kind == SymbolKind::Constant ? "constant" :
           kind == SymbolKind::Procedure ? "procedure" :
           kind == SymbolKind::Function ? "function" : "parameter";
}

void printSymbolEntryBrief(const SymbolEntry* entry) {
    if (entry == nullptr) {
        return;
    }

    std::cout << " [sym: {name=" << entry->name
              << ", kind=" << symbolKindName(entry->kind)
              << ", scope=" << entry->scopeLevel
              << ", type=" << dataTypeName(entry->type)
              << ", isArray=" << (entry->isArray ? "true" : "false")
              << ", isVarParam=" << (entry->isVarParam ? "true" : "false")
              << "}]";
    if (entry->isConstantLike()) {
        std::cout << " [constLiteral=" << entry->constLiteralText << "]";
    }
    if (entry->isArray) {
        std::cout << " [arrayBounds=";
        for (const auto& bound : entry->arrayBounds) {
            std::cout << "[" << bound.lower << ".." << bound.upper << "]";
        }
        std::cout << "]";
    }
}

}  // namespace

void printAstNode(const ASTNode *node, int indent) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    std::cout << nodeTypeName(node->nodeType);
    if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        std::cout << " (" << id->identifier << ")";
    }
    std::cout << std::endl;
    for (ASTNode* child : node->children) {
        printAstNode(child, indent + 1);
    }
}
void printAnnotatedAstNode(const ASTNode *node, int indent) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    std::cout << nodeTypeName(node->nodeType);
    if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        std::cout << " (" << id->identifier
                  << ", type: " << dataTypeName(id->dataType)
                  << ", isLValue: " << (id->isLValue ? "true" : "false") << ")";
    }
    std::cout << " (type: " << dataTypeName(node->dataType) << ")";
    printSymbolEntryBrief(node->symbolEntry);

    std::cout << std::endl;
    for (ASTNode* child : node->children) {
        printAnnotatedAstNode(child, indent + 1);
    }
}