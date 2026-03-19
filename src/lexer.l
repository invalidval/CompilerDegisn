%option noyywrap

%{
#include <cstring>
#include "parser.hpp"
%}

%%

[ \t\r\n]+                    ;
"program"                     return PROGRAM;
[a-zA-Z_][a-zA-Z0-9_]*        {
																yylval.text = strdup(yytext);
																return IDENTIFIER;
															}
.                             return yytext[0];

%%
