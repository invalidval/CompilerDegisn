#ifndef PASCC_PARSER_BRIDGE_H
#define PASCC_PARSER_BRIDGE_H

#include <cstdio>

class ProgramNode;
class ASTBuilder;

/*
 * Parser bridge owned by role A.
 * External modules (main/semantic/codegen) should use this header
 * instead of touching parser internals directly.
 */

ProgramNode* getParseResultRoot();
void resetParseResult();
ASTBuilder& getAstBuilder();
int getParseErrorCount();

int yyparse(void);
extern FILE* yyin;

#endif  // PASCC_PARSER_BRIDGE_H
