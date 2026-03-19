%{
#include <cstdio>
#include <cstdlib>
#include <string>

#include "ast.h"

static ASTBuilder g_astBuilder;
static ProgramNode* g_parseRoot = nullptr;

int yylex(void);
void yyerror(const char* s);
%}

%union {
    void* node;
    char* text;
}

%token PROGRAM
%token <text> IDENTIFIER

%type <node> program

%%

program:
      PROGRAM IDENTIFIER ';'
      {
          g_parseRoot = g_astBuilder.makeProgram(std::string($2));
                    $$ = static_cast<void*>(g_parseRoot);
          std::free($2);
      }
    | /* empty */
      {
          g_parseRoot = g_astBuilder.makeProgram("empty");
                    $$ = static_cast<void*>(g_parseRoot);
      }
    ;

%%

void yyerror(const char* s) {
    std::fprintf(stderr, "Parse error: %s\n", s);
}

ProgramNode* getParseResultRoot() {
    return g_parseRoot;
}

void resetParseResult() {
    g_parseRoot = nullptr;
}

ASTBuilder& getAstBuilder() {
    return g_astBuilder;
}
