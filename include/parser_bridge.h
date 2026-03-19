#ifndef PASCC_PARSER_BRIDGE_H
#define PASCC_PARSER_BRIDGE_H

class ProgramNode;
class ASTBuilder;

ProgramNode* getParseResultRoot();
void resetParseResult();
ASTBuilder& getAstBuilder();

#endif  // PASCC_PARSER_BRIDGE_H
