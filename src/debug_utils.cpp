#include "debug_utils.h"
#include "ast.h"
#include <iostream>
void printAstNode(const ASTNode *node, int indent) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    printf("%s", node->nodeType == NodeType::Program ? "Program" :
           node->nodeType == NodeType::Block ? "Block" :
           node->nodeType == NodeType::VarDecl ? "VarDecl" :
           node->nodeType == NodeType::ConstDecl ? "ConstDecl" :
           node->nodeType == NodeType::ProcDecl ? "ProcDecl" :
           node->nodeType == NodeType::FuncDecl ? "FuncDecl" :
           node->nodeType == NodeType::AssignStmt ? "AssignStmt" :
           node->nodeType == NodeType::IfStmt ? "IfStmt" :
           node->nodeType == NodeType::ForStmt ? "ForStmt" :
           node->nodeType == NodeType::CompoundStmt ? "CompoundStmt" :
           node->nodeType == NodeType::ProcCall ? "ProcCall" :
           node->nodeType == NodeType::BinaryExpr ? "BinaryExpr" :
           node->nodeType == NodeType::UnaryExpr ? "UnaryExpr" :
           node->nodeType == NodeType::Identifier ? "Identifier" :
           node->nodeType == NodeType::Literal ? "Literal" :
           node->nodeType == NodeType::ArrayAccess ? "ArrayAccess" :
           node->nodeType == NodeType::ArrayType ? "ArrayType" :
           node->nodeType == NodeType::ParamDecl ? "ParamDecl" :
           node->nodeType == NodeType::List ? "List" : "UnknownNode");
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
    printf("%s", node->nodeType == NodeType::Program ? "Program" :
           node->nodeType == NodeType::Block ? "Block" :
           node->nodeType == NodeType::VarDecl ? "VarDecl" :
           node->nodeType == NodeType::ConstDecl ? "ConstDecl" :
           node->nodeType == NodeType::ProcDecl ? "ProcDecl" :
           node->nodeType == NodeType::FuncDecl ? "FuncDecl" :
           node->nodeType == NodeType::AssignStmt ? "AssignStmt" :
           node->nodeType == NodeType::IfStmt ? "IfStmt" :
           node->nodeType == NodeType::ForStmt ? "ForStmt" :
           node->nodeType == NodeType::CompoundStmt ? "CompoundStmt" :
           node->nodeType == NodeType::ProcCall ? "ProcCall" :
           node->nodeType == NodeType::BinaryExpr ? "BinaryExpr" :
           node->nodeType == NodeType::UnaryExpr ? "UnaryExpr" :
           node->nodeType == NodeType::Identifier ? "Identifier" :
           node->nodeType == NodeType::Literal ? "Literal" :
           node->nodeType == NodeType::ArrayAccess ? "ArrayAccess" :
           node->nodeType == NodeType::ArrayType ? "ArrayType" :
           node->nodeType == NodeType::ParamDecl ? "ParamDecl" :
           node->nodeType == NodeType::List ? "List" : "UnknownNode");
    if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        std::cout << " (" << id->identifier << ", type: " << static_cast<int>(id->dataType) << ", isLValue: " << id->isLValue << ")";
    }
    

    std::cout << std::endl;
    for (ASTNode* child : node->children) {
        printAnnotatedAstNode(child, indent + 1);
    }
}