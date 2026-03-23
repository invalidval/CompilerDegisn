#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H
#include "ast.h"
void printAstNode(const ASTNode *node, int indent = 0);
void printAnnotatedAstNode(const ASTNode *node, int indent = 0);
#endif // DEBUG_UTILS_H