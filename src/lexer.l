%option noyywrap nodefault

%{
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "error_handler.h"
#include "parser.hpp"

namespace {

int g_line = 1;
int g_column = 1;
int g_token_line = 1;
int g_token_column = 1;
int g_comment_start_line = 1;
int g_comment_start_column = 1;
bool g_in_comment = false;
int g_comment_depth = 0;
int g_directive_start_line = 1;
int g_directive_start_column = 1;
bool g_in_directive = false;
std::string g_directive_body;

std::string g_last_lexeme;
std::string g_last_type;
std::string g_last_rule;
std::vector<CompileError> g_lex_errors;
size_t g_strdup_count = 0;
int g_token_count = 0;

constexpr int INITIAL_COMMENT_DEPTH = 1;
constexpr size_t MAX_IDENTIFIER_LENGTH = 255;
constexpr size_t MAX_STRING_LENGTH = 1024;

const std::unordered_map<std::string, int> g_keyword_tokens = {
	{"program", PROGRAM}, {"const", CONST}, {"var", VAR}, {"integer", INTEGER},
	{"real", REAL}, {"boolean", BOOLEAN}, {"char", CHAR}, {"array", ARRAY},
	{"of", OF}, {"function", FUNCTION}, {"procedure", PROCEDURE},
	{"begin", KW_BEGIN}, {"end", KW_END}, {"if", IF}, {"then", THEN},
	{"else", ELSE}, {"while", WHILE}, {"do", DO}, {"for", FOR},
	{"to", TO}, {"downto", DOWNTO}, {"read", READ}, {"write", WRITE},
	{"not", NOT}, {"and", AND}, {"or", OR}, {"div", DIV}, {"mod", MOD},
	{"true", TRUE}, {"false", FALSE}
};

void advancePosition(const char* text, int len) {
	for (int i = 0; i < len; ++i) {
		if (text[i] == '\n') {
			++g_line;
			g_column = 1;
		} else {
			++g_column;
		}
	}
}

std::string toLower(const char* text, int len) {
	std::string out;
	out.reserve(static_cast<size_t>(len));
	for (int i = 0; i < len; ++i) {
		out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(text[i]))));
	}
	return out;
}

char* duplicateText(const char* text) {
	++g_strdup_count;
	return strdup(text);
}

bool isNumericOverflow(const char* text, int len) {
	if (len <= 0) {
		return false;
	}
	if ((len >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) || text[0] == '$') {
		return false;
	}

	std::string literal(text, static_cast<size_t>(len));
	errno = 0;
	char* end = nullptr;
	double value = std::strtod(literal.c_str(), &end);
	if (end == nullptr || end != literal.c_str() + literal.size()) {
		return false;
	}
	if (errno == ERANGE && std::isinf(value)) {
		return true;
	}
	return false;
}

std::string trim(const std::string& text) {
	size_t left = 0;
	while (left < text.size() && std::isspace(static_cast<unsigned char>(text[left]))) {
		++left;
	}
	size_t right = text.size();
	while (right > left && std::isspace(static_cast<unsigned char>(text[right - 1]))) {
		--right;
	}
	return text.substr(left, right - left);
}

bool isValidDirective(const std::string& body) {
	std::string cleaned = trim(body);
	if (cleaned.empty()) {
		return false;
	}

	std::string head;
	for (char ch : cleaned) {
		if (std::isspace(static_cast<unsigned char>(ch))) {
			break;
		}
		head.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
	}

	return head == "IFDEF" || head == "UNDEF" || head == "DEFINE" || head == "ENDIF" || head == "ELSE";
}

void setTokenMeta(const char* type, const char* rule) {
	g_last_lexeme.assign(yytext, static_cast<size_t>(yyleng));
	g_last_type = type;
	g_last_rule = rule;
	++g_token_count;
}

void reportLexError(int line, int column, const std::string& reason, const std::string& lexeme) {
	g_lex_errors.push_back({line, column, "Lexical error: " + reason + " (lexeme='" + lexeme + "')"});
}

}  // namespace

#define YY_USER_ACTION \
	do { \
		g_token_line = g_line; \
		g_token_column = g_column; \
		yylloc.first_line = g_token_line; \
		yylloc.first_column = g_token_column; \
		advancePosition(yytext, yyleng); \
		yylloc.last_line = g_line; \
		yylloc.last_column = g_column; \
	} while (0);
%}

%x COMMENT DIRECTIVE

DIGIT      [0-9]
ID_START   [a-zA-Z_]
ID_CONT    [a-zA-Z0-9_]
WS         [ \t\r]+

%%

{WS}                                  ;
\n                                    ;

<INITIAL>"\xEF\xBB\xBF"               {
										if (g_token_line != 1 || g_token_column != 1) {
											reportLexError(g_token_line, g_token_column,
												"illegal character", yytext);
										}
									  }

<INITIAL>"{$"                          {
										g_directive_start_line = g_token_line;
										g_directive_start_column = g_token_column;
										g_in_directive = true;
										g_directive_body.clear();
										BEGIN(DIRECTIVE);
									  }

<INITIAL>"{"                           {
										if (!g_in_comment) {
											g_comment_start_line = g_token_line;
											g_comment_start_column = g_token_column;
											g_in_comment = true;
											g_comment_depth = INITIAL_COMMENT_DEPTH;
											BEGIN(COMMENT);
										}
									  }

<COMMENT>"{"                           {
										++g_comment_depth;
									  }

<DIRECTIVE>[^}\n]+                     {
										g_directive_body.append(yytext, static_cast<size_t>(yyleng));
									  }

<DIRECTIVE>\n                           {
										// Line count already updated by YY_USER_ACTION
										g_directive_body.push_back('\n');
									  }

<DIRECTIVE>"}"                          {
										if (!isValidDirective(g_directive_body)) {
											reportLexError(g_directive_start_line, g_directive_start_column,
												"invalid directive", "{$" + trim(g_directive_body) + "}");
										}
										g_directive_body.clear();
										g_in_directive = false;
										BEGIN(INITIAL);
									  }

<COMMENT>[^{}\n]+                      ;

<COMMENT>\n                            {
										// Line count already updated by YY_USER_ACTION
									  }

<COMMENT>"}"                           {
										if (g_in_comment) {
											--g_comment_depth;
											if (g_comment_depth == 0) {
												g_in_comment = false;
												BEGIN(INITIAL);
											}
										} else {
											return yytext[0];
										}
									  }

<INITIAL>"}"                           {
										return yytext[0];
									  }

<<EOF>>                               {
										if (g_in_comment) {
											reportLexError(g_comment_start_line, g_comment_start_column,
												"unterminated comment", "{");
											g_in_comment = false;
											g_comment_depth = 0;
										}
										if (g_in_directive) {
											reportLexError(g_directive_start_line, g_directive_start_column,
												"unterminated directive", "{$");
											g_in_directive = false;
											g_directive_body.clear();
										}
										g_last_lexeme.clear();
										g_last_type = "EndOfFile";
										g_last_rule = "eof";
										return 0;
									  }

\'([^\'\\\n]|\\[ntr0\'\\])\'            {
										if (!g_in_comment) {
											setTokenMeta("Character", "char_literal");
											yylval.text = duplicateText(yytext);
											return CHARACTER;
										}
									  }

\'([^\'\\\n]|\\.)+\'                  {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"malformed character literal", yytext);
											continue;
										}
									  }

\'([^\'\\\n]|\\.)*                    {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"unterminated character literal", yytext);
											continue;
										}
									  }

\"([^\"\\\n]|\\[ntr0\"\\])*\"        {
										if (!g_in_comment) {
											if (static_cast<size_t>(yyleng) > MAX_STRING_LENGTH + 2) {
												reportLexError(g_token_line, g_token_column,
													"string literal too long", yytext);
												continue;
											}
											setTokenMeta("String", "string_literal");
											yylval.text = duplicateText(yytext);
											return STRING;
										}
									  }

\"([^\"\\\n]|\\.)*                    {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"unterminated string literal", yytext);
											continue;
										}
									  }

0[xX][0-9A-Fa-f]+                     {
										if (!g_in_comment) {
											setTokenMeta("Number", "hex_0x");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

\$[0-9A-Fa-f]+                        {
										if (!g_in_comment) {
											setTokenMeta("Number", "hex_dollar");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

0[xX][0-9A-Za-z_]*                    {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"malformed number", yytext);
											continue;
										}
									  }

\$[0-9A-Za-z_]+                       {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"malformed number", yytext);
											continue;
										}
									  }

{DIGIT}+\.{DIGIT}+([eE][+-]?{DIGIT}+)? {
										if (!g_in_comment) {
											if (isNumericOverflow(yytext, yyleng)) {
												reportLexError(g_token_line, g_token_column,
													"numeric literal overflow", yytext);
												continue;
											}
											setTokenMeta("Number", "real_number");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

\.{DIGIT}+([eE][+-]?{DIGIT}+)?         {
										if (!g_in_comment) {
											if (isNumericOverflow(yytext, yyleng)) {
												reportLexError(g_token_line, g_token_column,
													"numeric literal overflow", yytext);
												continue;
											}
											setTokenMeta("Number", "leading_dot_real");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

{DIGIT}+\.{DIGIT}+\.{DIGIT}*([0-9.]*) {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"malformed number", yytext);
											continue;
										}
									  }

{DIGIT}+[eE][+-]?{DIGIT}+              {
										if (!g_in_comment) {
											if (isNumericOverflow(yytext, yyleng)) {
												reportLexError(g_token_line, g_token_column,
													"numeric literal overflow", yytext);
												continue;
											}
											setTokenMeta("Number", "scientific_number");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

{DIGIT}+[eE][+-]?                     {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"malformed number", yytext);
											continue;
										}
									  }

{DIGIT}+                              {
										if (!g_in_comment) {
											setTokenMeta("Number", "integer_number");
											yylval.text = duplicateText(yytext);
											return NUMBER;
										}
									  }

":="                                  {
										if (!g_in_comment) {
											setTokenMeta("Operator", "assign_operator");
											return ASSIGN;
										}
									 }

"<="                                  {
										if (!g_in_comment) {
											setTokenMeta("Operator", "le_operator");
											return LE;
										}
									 }

">="                                  {
										if (!g_in_comment) {
											setTokenMeta("Operator", "ge_operator");
											return GE;
										}
									 }

"<>"                                  {
										if (!g_in_comment) {
											setTokenMeta("Operator", "ne_operator");
											return NE;
										}
									 }

".."                                  {
										if (!g_in_comment) {
											setTokenMeta("Operator", "dotdot_operator");
											return DOTDOT;
										}
									 }

{ID_START}{ID_CONT}*                  {
										if (!g_in_comment) {
											if (static_cast<size_t>(yyleng) > MAX_IDENTIFIER_LENGTH) {
												reportLexError(g_token_line, g_token_column,
													"identifier too long", yytext);
												continue;
											}
											std::string lowered = toLower(yytext, yyleng);
											auto it = g_keyword_tokens.find(lowered);
											if (it != g_keyword_tokens.end()) {
												setTokenMeta("Keyword", "keyword_identifier");
												yylval.text = duplicateText(yytext);
												return it->second;
											}
											setTokenMeta("Identifier", "identifier");
											yylval.text = duplicateText(lowered.c_str());
											return IDENTIFIER;
										}
									  }

[:;,.()\[\]]                          {
										if (!g_in_comment) {
											setTokenMeta("Delimiter", "delimiter");
											return yytext[0];
										}
									  }

[+\-*/=<>]                            {
										if (!g_in_comment) {
											setTokenMeta("Operator", "single_operator");
											return yytext[0];
										}
									  }

.                                     {
										if (!g_in_comment) {
											reportLexError(g_token_line, g_token_column,
												"illegal character", yytext);
											continue;
										}
									  }

%%

void lexerResetState() {
	g_line = 1;
	g_column = 1;
	g_token_line = 1;
	g_token_column = 1;
	g_comment_start_line = 1;
	g_comment_start_column = 1;
	g_comment_depth = 0;
	g_directive_start_line = 1;
	g_directive_start_column = 1;
	g_directive_body.clear();
	g_last_lexeme.clear();
	g_last_type.clear();
	g_last_rule.clear();
	g_lex_errors.clear();
	g_strdup_count = 0;
	g_token_count = 0;
	g_in_comment = false;
	g_in_directive = false;
}

const char* lexerLastLexeme() {
	return g_last_lexeme.c_str();
}

const char* lexerLastType() {
	return g_last_type.c_str();
}

const char* lexerLastRule() {
	return g_last_rule.c_str();
}

int lexerLastLine() {
	return g_token_line;
}

int lexerLastColumn() {
	return g_token_column;
}

void lexerClearErrors() {
	g_lex_errors.clear();
}

int lexerErrorCount() {
	return static_cast<int>(g_lex_errors.size());
}

const CompileError* lexerErrorAt(int index) {
	if (index < 0 || index >= static_cast<int>(g_lex_errors.size())) {
		return nullptr;
	}
	return &g_lex_errors[static_cast<size_t>(index)];
}

int lexerAllocatedStringCount() {
	return static_cast<int>(g_strdup_count);
}

int lexerTokenCount() {
	return g_token_count;
}
