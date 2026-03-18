%{
#include <cstdio>

int yylex(void);
void yyerror(const char* s);
%}

%token PROGRAM IDENTIFIER

%%

program:
      /* empty */
    ;

%%

void yyerror(const char* s) {
    std::fprintf(stderr, "Parse error: %s\n", s);
}
