#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H
#include <string>

#include "ast.h"

enum class PasccLogLevel {
	Off = 0,
	Error = 1,
	Warn = 2,
	Info = 3,
	Debug = 4,
};

bool pasccLogEnabled(PasccLogLevel level);
void pasccLog(const std::string& stage, PasccLogLevel level, const std::string& message);
const char* pasccLogLevelName(PasccLogLevel level);

void printAstNode(const ASTNode *node, int indent = 0);
void printAnnotatedAstNode(const ASTNode *node, int indent = 0);
#endif // DEBUG_UTILS_H