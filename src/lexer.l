%option noyywrap

%{
#include "parser.hpp"
%}

%%

[ \t\r\n]+                    ;
"program"                     return PROGRAM;
[a-zA-Z_][a-zA-Z0-9_]*        return IDENTIFIER;
.                             return yytext[0];

%%
