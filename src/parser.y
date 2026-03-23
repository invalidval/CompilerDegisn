%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ast.h"

static ASTBuilder g_astBuilder;
static ProgramNode* g_parseRoot = nullptr;
static int g_rule_line = 1;
static int g_rule_column = 1;
static int g_parse_error_count = 0;

// Keep currentPos() aligned with the start location of the current reduction.
#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(Current, Rhs, N)                                           \
        do {                                                                          \
                if (N) {                                                                  \
                        (Current).first_line = YYRHSLOC(Rhs, 1).first_line;                  \
                        (Current).first_column = YYRHSLOC(Rhs, 1).first_column;              \
                        (Current).last_line = YYRHSLOC(Rhs, N).last_line;                     \
                        (Current).last_column = YYRHSLOC(Rhs, N).last_column;                 \
                } else {                                                                  \
                        (Current).first_line = (Current).last_line = YYRHSLOC(Rhs, 0).last_line;     \
                        (Current).first_column = (Current).last_column = YYRHSLOC(Rhs, 0).last_column; \
                }                                                                         \
                g_rule_line = (Current).first_line;                                       \
                g_rule_column = (Current).first_column;                                   \
        } while (0)
#endif

int yylex(void);
void yyerror(const char* s);

int lexerLastLine();
int lexerLastColumn();
const char* lexerLastLexeme();

namespace {

SourcePos currentPos() {
        return SourcePos{g_rule_line, g_rule_column};
}

ListNode* asList(void* node) {
    return static_cast<ListNode*>(node);
}

ASTNode* asNode(void* node) {
    return static_cast<ASTNode*>(node);
}

std::vector<ASTNode*> listItems(ASTNode* node) {
    if (node == nullptr) {
        return {};
    }
    if (node->nodeType == NodeType::List) {
        return static_cast<ListNode*>(node)->children;
    }
    return {node};
}

DataType dataTypeFromBasicTypeNode(ASTNode* node) {
    if (node == nullptr || node->nodeType != NodeType::Identifier) {
        return DataType::Unknown;
    }
    const std::string& t = static_cast<IdentifierNode*>(node)->identifier;
    if (t == "integer") return DataType::Integer;
    if (t == "real") return DataType::Real;
    if (t == "boolean") return DataType::Boolean;
    if (t == "char") return DataType::Char;
    return DataType::Unknown;
}

ASTNode* buildArrayTypeFromRanges(ASTNode* rangesNode, ASTNode* elemType, SourcePos pos) {
    if (rangesNode == nullptr || rangesNode->nodeType != NodeType::List) {
        return elemType;
    }

    auto* ranges = static_cast<ListNode*>(rangesNode);
    ASTNode* current = elemType;
    for (int i = static_cast<int>(ranges->children.size()) - 1; i >= 0; --i) {
        ASTNode* seg = ranges->children[static_cast<size_t>(i)];
        if (seg == nullptr || seg->nodeType != NodeType::ArrayType || seg->children.size() < 2) {
            continue;
        }
        current = g_astBuilder.makeArrayType(seg->children[0], seg->children[1], current, pos);
    }
    return current;
}

ASTNode* buildArrayAccessFromIndices(ASTNode* base, ASTNode* indicesNode, SourcePos pos) {
    if (indicesNode == nullptr || indicesNode->nodeType != NodeType::List) {
        return base;
    }

    ASTNode* current = base;
    auto* list = static_cast<ListNode*>(indicesNode);
    for (ASTNode* idx : list->children) {
        current = g_astBuilder.makeArrayAccess(current, idx, 0, pos);
    }
    return current;
}

}  // namespace
%}

%union {
    void* node;
    char* text;
}

/* Keywords */
%token PROGRAM CONST VAR INTEGER REAL BOOLEAN CHAR ARRAY OF FUNCTION PROCEDURE
%token KW_BEGIN KW_END IF THEN ELSE WHILE DO FOR TO DOWNTO READ WRITE NOT AND OR DIV MOD

/* Literals */
%token <text> NUMBER CHARACTER STRING
%token <text> IDENTIFIER

%destructor { std::free($$); } <text>

%locations

/* Operators and special symbols (multi-character) */
%token ASSIGN LE GE NE DOTDOT

/* Single-character operators return their ASCII value. */

%type <node> program program_body subprogram_body
%type <node> opt_program_input idlist
%type <node> const_declarations const_declaration_list const_declaration const_value
%type <node> var_declarations var_declaration_list var_declaration type basic_type period range
%type <node> subprogram_declarations subprogram
%type <node> formal_parameter parameter_list parameter var_parameter value_parameter
%type <node> compound_statement statement_list statement
%type <node> variable id_varpart procedure_call expression_list variable_list
%type <node> expression simple_expression term factor

%right ASSIGN
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left OR
%left AND
%left '=' NE '<' '>' LE GE
%left '+' '-'
%left '*' '/' DIV MOD
%right NOT
%right UMINUS

%%

program:
            PROGRAM IDENTIFIER opt_program_input ';' program_body '.'
      {
                    auto* root = g_astBuilder.makeProgram(std::string($2), currentPos());
                    root->children.push_back(asNode($5));
                    if ($3 != nullptr) {
                            root->children.push_back(asNode($3));
                    }
                    g_parseRoot = root;
                    $$ = static_cast<void*>(g_parseRoot);
          std::free($2);
      }
        ;

opt_program_input:
            /* empty */
      {
                    $$ = nullptr;
            }
        | '(' idlist ')'
            {
                    $$ = $2;
            }
        ;

program_body:
            const_declarations var_declarations subprogram_declarations compound_statement
            {
                    auto* block = g_astBuilder.makeBlock(currentPos());
                    block->children.push_back(asNode($1));
                    block->children.push_back(asNode($2));
                    block->children.push_back(asNode($3));
                    block->children.push_back(asNode($4));
                    $$ = static_cast<void*>(block);
            }
        ;

const_declarations:
            /* empty */
            {
                    $$ = static_cast<void*>(g_astBuilder.makeList(ListKind::Declarations, currentPos()));
            }
        | CONST const_declaration_list ';'
            {
                    $$ = $2;
            }
        ;

const_declaration_list:
            const_declaration
            {
                    auto* list = g_astBuilder.makeList(ListKind::Declarations, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | const_declaration_list ';' const_declaration
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

const_declaration:
            IDENTIFIER '=' const_value
            {
                    auto* id = g_astBuilder.makeIdentifier(std::string($1), currentPos());
                    auto* decl = g_astBuilder.makeConstDecl(id, asNode($3), currentPos());
                    $$ = static_cast<void*>(decl);
                    std::free($1);
            }
        ;

const_value:
            NUMBER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | CHARACTER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | STRING
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | '+' NUMBER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($2), currentPos()));
                    std::free($2);
            }
        | '-' NUMBER
            {
                    auto* lit = g_astBuilder.makeLiteral(std::string($2), currentPos());
                    $$ = static_cast<void*>(g_astBuilder.makeUnaryExpr("-", lit, currentPos()));
                    std::free($2);
            }
        ;

var_declarations:
            /* empty */
            {
                    $$ = static_cast<void*>(g_astBuilder.makeList(ListKind::Declarations, currentPos()));
            }
        | VAR var_declaration_list ';'
            {
                    $$ = $2;
            }
        ;

var_declaration_list:
            var_declaration
            {
                    auto* list = g_astBuilder.makeList(ListKind::Declarations, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | var_declaration_list ';' var_declaration
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

var_declaration:
            idlist ':' type
            {
                    $$ = static_cast<void*>(g_astBuilder.makeVarDecl(asNode($1), asNode($3), currentPos()));
            }
        ;

idlist:
            IDENTIFIER
            {
                    auto* list = g_astBuilder.makeList(ListKind::Identifiers, currentPos());
                    list->add(g_astBuilder.makeIdentifier(std::string($1), currentPos()));
                    $$ = static_cast<void*>(list);
                    std::free($1);
            }
        | idlist ',' IDENTIFIER
            {
                    auto* list = asList($1);
                    list->add(g_astBuilder.makeIdentifier(std::string($3), currentPos()));
                    $$ = $1;
                    std::free($3);
            }
        ;

type:
            basic_type
            {
                    $$ = $1;
            }
        | ARRAY '[' period ']' OF basic_type
            {
                    $$ = static_cast<void*>(buildArrayTypeFromRanges(asNode($3), asNode($6), currentPos()));
            }
        ;

basic_type:
            INTEGER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIdentifier("integer", currentPos()));
            }
        | REAL
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIdentifier("real", currentPos()));
            }
        | BOOLEAN
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIdentifier("boolean", currentPos()));
            }
        | CHAR
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIdentifier("char", currentPos()));
            }
        ;

period:
            range
            {
                    auto* list = g_astBuilder.makeList(ListKind::ArrayRanges, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | period ',' range
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

range:
            NUMBER DOTDOT NUMBER
            {
                    auto* lower = g_astBuilder.makeLiteral(std::string($1), currentPos());
                    auto* upper = g_astBuilder.makeLiteral(std::string($3), currentPos());
                    $$ = static_cast<void*>(g_astBuilder.makeArrayType(lower, upper, nullptr, currentPos()));
                    std::free($1);
                    std::free($3);
            }
        ;

subprogram_declarations:
            /* empty */
            {
                    $$ = static_cast<void*>(g_astBuilder.makeList(ListKind::Declarations, currentPos()));
            }
        | subprogram_declarations subprogram ';'
            {
                    auto* list = asList($1);
                    list->add(asNode($2));
                    $$ = $1;
            }
        ;

subprogram:
            PROCEDURE IDENTIFIER formal_parameter ';' subprogram_body
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcDecl(std::string($2), asNode($3), asNode($5), currentPos()));
                    std::free($2);
            }
        | FUNCTION IDENTIFIER formal_parameter ':' basic_type ';' subprogram_body
            {
                    DataType ret = dataTypeFromBasicTypeNode(asNode($5));
                    $$ = static_cast<void*>(g_astBuilder.makeFuncDecl(std::string($2), asNode($3), ret, asNode($7), currentPos()));
                    std::free($2);
            }
        ;

formal_parameter:
            /* empty */
            {
                    $$ = static_cast<void*>(g_astBuilder.makeList(ListKind::Parameters, currentPos()));
            }
        | '(' parameter_list ')'
            {
                    $$ = $2;
            }
        ;

parameter_list:
            parameter
            {
                    auto* list = g_astBuilder.makeList(ListKind::Parameters, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | parameter_list ';' parameter
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

parameter:
            var_parameter
            {
                    $$ = $1;
            }
        | value_parameter
            {
                    $$ = $1;
            }
        ;

var_parameter:
            VAR value_parameter
            {
                    auto* p = static_cast<ParamDeclNode*>(asNode($2));
                    $$ = static_cast<void*>(g_astBuilder.makeParamDecl(true, p->children[0], p->children[1], p->pos));
            }
        ;

value_parameter:
            idlist ':' basic_type
            {
                    $$ = static_cast<void*>(g_astBuilder.makeParamDecl(false, asNode($1), asNode($3), currentPos()));
            }
        ;

subprogram_body:
            const_declarations var_declarations compound_statement
            {
                    auto* block = g_astBuilder.makeBlock(currentPos());
                    block->children.push_back(asNode($1));
                    block->children.push_back(asNode($2));
                    block->children.push_back(asNode($3));
                    $$ = static_cast<void*>(block);
            }
        ;

compound_statement:
            KW_BEGIN statement_list KW_END
            {
                    auto* stmtList = asList($2);
                    $$ = static_cast<void*>(g_astBuilder.makeCompoundStmt(stmtList->children, currentPos()));
            }
        ;

statement_list:
            /* empty */
            {
                    $$ = static_cast<void*>(g_astBuilder.makeList(ListKind::Statements, currentPos()));
            }
        | statement
            {
                    auto* list = g_astBuilder.makeList(ListKind::Statements, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | statement_list ';' statement
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        | statement_list ';' error
            {
                    // Panic-mode recovery: skip one malformed statement and continue.
                    yyerrok;
                                        yyclearin;
                    $$ = $1;
            }
        ;

statement:
            /* empty */
            {
                    $$ = nullptr;
            }
        | variable ASSIGN expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeAssignStmt(asNode($1), asNode($3), currentPos()));
            }
        | procedure_call
            {
                    $$ = $1;
            }
        | compound_statement
            {
                    $$ = $1;
            }
        | IF expression THEN statement ELSE statement
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIfStmt(asNode($2), asNode($4), asNode($6), currentPos()));
            }
        | IF expression THEN statement %prec LOWER_THAN_ELSE
            {
                    $$ = static_cast<void*>(g_astBuilder.makeIfStmt(asNode($2), asNode($4), nullptr, currentPos()));
            }
        | FOR IDENTIFIER ASSIGN expression TO expression DO statement
            {
                    auto* id = g_astBuilder.makeIdentifier(std::string($2), currentPos());
                    $$ = static_cast<void*>(g_astBuilder.makeForStmt(id, asNode($4), asNode($6), asNode($8), false, currentPos()));
                    std::free($2);
            }
        | FOR IDENTIFIER ASSIGN expression DOWNTO expression DO statement
            {
                    auto* id = g_astBuilder.makeIdentifier(std::string($2), currentPos());
                    $$ = static_cast<void*>(g_astBuilder.makeForStmt(id, asNode($4), asNode($6), asNode($8), true, currentPos()));
                    std::free($2);
            }
        | WHILE expression DO statement
            {
                    $$ = static_cast<void*>(g_astBuilder.makeWhileStmt(asNode($2), asNode($4), currentPos()));
            }
        | READ '(' variable_list ')'
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcCall("read", listItems(asNode($3)), currentPos()));
            }
        | WRITE '(' expression_list ')'
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcCall("write", listItems(asNode($3)), currentPos()));
            }
        ;

variable:
            IDENTIFIER id_varpart
            {
                    auto* id = g_astBuilder.makeIdentifier(std::string($1), currentPos());
                    $$ = static_cast<void*>(buildArrayAccessFromIndices(id, asNode($2), currentPos()));
                    std::free($1);
            }
        ;

id_varpart:
            /* empty */
            {
                    $$ = nullptr;
            }
        | '[' expression_list ']'
            {
                    $$ = $2;
            }
        ;

procedure_call:
            IDENTIFIER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcCall(std::string($1), {}, currentPos()));
                    std::free($1);
            }
        | IDENTIFIER '(' expression_list ')'
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcCall(std::string($1), listItems(asNode($3)), currentPos()));
                    std::free($1);
            }
        ;

expression_list:
            expression
            {
                    auto* list = g_astBuilder.makeList(ListKind::Expressions, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | expression_list ',' expression
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

variable_list:
            variable
            {
                    auto* list = g_astBuilder.makeList(ListKind::Expressions, currentPos());
                    list->add(asNode($1));
                    $$ = static_cast<void*>(list);
            }
        | variable_list ',' variable
            {
                    auto* list = asList($1);
                    list->add(asNode($3));
                    $$ = $1;
            }
        ;

expression:
            simple_expression
            {
                    $$ = $1;
            }
        | simple_expression '=' simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("=", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression NE simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("<>", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression '<' simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("<", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression LE simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("<=", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression '>' simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr(">", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression GE simple_expression
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr(">=", asNode($1), asNode($3), currentPos()));
            }
        ;

simple_expression:
            term
            {
                    $$ = $1;
            }
        | simple_expression '+' term
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("+", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression '-' term
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("-", asNode($1), asNode($3), currentPos()));
            }
        | simple_expression OR term
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("or", asNode($1), asNode($3), currentPos()));
            }
        ;

term:
            factor
            {
                    $$ = $1;
            }
        | term '*' factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("*", asNode($1), asNode($3), currentPos()));
            }
        | term '/' factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("/", asNode($1), asNode($3), currentPos()));
            }
        | term DIV factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("div", asNode($1), asNode($3), currentPos()));
            }
        | term MOD factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("mod", asNode($1), asNode($3), currentPos()));
            }
        | term AND factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeBinaryExpr("and", asNode($1), asNode($3), currentPos()));
            }
        ;

factor:
            NUMBER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | CHARACTER
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | STRING
            {
                    $$ = static_cast<void*>(g_astBuilder.makeLiteral(std::string($1), currentPos()));
                    std::free($1);
            }
        | variable
            {
                    $$ = $1;
            }
        | '(' expression ')'
            {
                    $$ = $2;
            }
        | IDENTIFIER '(' expression_list ')'
            {
                    $$ = static_cast<void*>(g_astBuilder.makeProcCall(std::string($1), listItems(asNode($3)), currentPos()));
                    std::free($1);
            }
        | NOT factor
            {
                    $$ = static_cast<void*>(g_astBuilder.makeUnaryExpr("not", asNode($2), currentPos()));
            }
        | '-' factor %prec UMINUS
            {
                    $$ = static_cast<void*>(g_astBuilder.makeUnaryExpr("-", asNode($2), currentPos()));
            }
        ;

%%

void yyerror(const char* s) {
        ++g_parse_error_count;
        const char* lexeme = lexerLastLexeme();
        if (lexeme == nullptr || std::strlen(lexeme) == 0) {
                std::fprintf(stderr, "Parse error at %d:%d: %s\n",
                                         yylloc.first_line, yylloc.first_column, s);
        } else {
                std::fprintf(stderr, "Parse error at %d:%d near '%s': %s\n",
                                         yylloc.first_line, yylloc.first_column, lexeme, s);
        }
}

ProgramNode* getParseResultRoot() {
    return g_parseRoot;
}

void resetParseResult() {
    g_parseRoot = nullptr;
        g_astBuilder = ASTBuilder();
        g_rule_line = 1;
        g_rule_column = 1;
        g_parse_error_count = 0;
}

ASTBuilder& getAstBuilder() {
    return g_astBuilder;
}

int getParseErrorCount() {
        return g_parse_error_count;
}
