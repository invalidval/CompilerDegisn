#include <cstdio>

#include "ast.h"
#include "parser_bridge.h"
#include "error_handler.h"
#include "semantic_annotator.h"

namespace {

ASTBuilder g_astBuilder;
ProgramNode* g_parseRoot = nullptr;

ProgramNode* buildFixedSemanticAstRoot() {
	ProgramNode* program = g_astBuilder.makeProgram("example");

	// program example(input, output);
	ListNode* ioList = g_astBuilder.makeList(ListKind::Identifiers);
	ioList->add(g_astBuilder.makeIdentifier("input"));
	ioList->add(g_astBuilder.makeIdentifier("output"));

	// Global block: const/var/subprogram/compound
	BlockNode* globalBlock = g_astBuilder.makeBlock();
	ListNode* globalConstDecls = g_astBuilder.makeList(ListKind::Declarations);
	ListNode* globalVarDecls = g_astBuilder.makeList(ListKind::Declarations);
	ListNode* globalSubprogramDecls = g_astBuilder.makeList(ListKind::Declarations);

	// var x, y: integer;
	ListNode* xyIds = g_astBuilder.makeList(ListKind::Identifiers);
	xyIds->add(g_astBuilder.makeIdentifier("x"));
	xyIds->add(g_astBuilder.makeIdentifier("y"));
	globalVarDecls->add(g_astBuilder.makeVarDecl(xyIds, g_astBuilder.makeIdentifier("integer")));

	// function gcd(a, b: integer): integer;
	ListNode* gcdParamIds = g_astBuilder.makeList(ListKind::Identifiers);
	gcdParamIds->add(g_astBuilder.makeIdentifier("a"));
	gcdParamIds->add(g_astBuilder.makeIdentifier("b"));
	ParamDeclNode* gcdParamDecl =
		g_astBuilder.makeParamDecl(false, gcdParamIds, g_astBuilder.makeIdentifier("integer"));
	ListNode* gcdParams = g_astBuilder.makeList(ListKind::Parameters);
	gcdParams->add(gcdParamDecl);

	// gcd body:
	// begin
	//   if b=0 then gcd:=a else gcd:=gcd(b, a mod b)
	// end
	BlockNode* gcdBody = g_astBuilder.makeBlock();
	ListNode* gcdConstDecls = g_astBuilder.makeList(ListKind::Declarations);
	ListNode* gcdVarDecls = g_astBuilder.makeList(ListKind::Declarations);

	BinaryExprNode* cond = g_astBuilder.makeBinaryExpr(
		"=",
		g_astBuilder.makeIdentifier("b"),
		g_astBuilder.makeLiteral("0")
	);

	AssignStmtNode* thenAssign = g_astBuilder.makeAssignStmt(
		g_astBuilder.makeIdentifier("gcd"),
		g_astBuilder.makeIdentifier("a")
	);

	BinaryExprNode* modExpr = g_astBuilder.makeBinaryExpr(
		"mod",
		g_astBuilder.makeIdentifier("a"),
		g_astBuilder.makeIdentifier("b")
	);
	ProcCallNode* recursiveCall = g_astBuilder.makeProcCall(
		"gcd",
		{g_astBuilder.makeIdentifier("b"), modExpr}
	);
	AssignStmtNode* elseAssign = g_astBuilder.makeAssignStmt(
		g_astBuilder.makeIdentifier("gcd"),
		recursiveCall
	);

	IfStmtNode* ifStmt = g_astBuilder.makeIfStmt(cond, thenAssign, elseAssign);
	CompoundStmtNode* gcdCompound = g_astBuilder.makeCompoundStmt({ifStmt});

	gcdBody->children.push_back(gcdConstDecls);
	gcdBody->children.push_back(gcdVarDecls);
	gcdBody->children.push_back(gcdCompound);

	FuncDeclNode* gcdFunc = g_astBuilder.makeFuncDecl("gcd", gcdParams, DataType::Integer, gcdBody);
	globalSubprogramDecls->add(gcdFunc);

	// Main body:
	// begin
	//   read(x, y);
	//   write(gcd(x, y))
	// end
	ProcCallNode* readCall = g_astBuilder.makeProcCall(
		"read",
		{g_astBuilder.makeIdentifier("x"), g_astBuilder.makeIdentifier("y")}
	);
	ProcCallNode* gcdCallInWrite = g_astBuilder.makeProcCall(
		"gcd",
		{g_astBuilder.makeIdentifier("x"), g_astBuilder.makeIdentifier("y")}
	);
	ProcCallNode* writeCall = g_astBuilder.makeProcCall("write", {gcdCallInWrite});
	CompoundStmtNode* mainCompound = g_astBuilder.makeCompoundStmt({readCall, writeCall});

	globalBlock->children.push_back(globalConstDecls);
	globalBlock->children.push_back(globalVarDecls);
	globalBlock->children.push_back(globalSubprogramDecls);
	globalBlock->children.push_back(mainCompound);

	program->children.push_back(globalBlock);
	program->children.push_back(ioList);
	return program;
}

}  // namespace

// Parser stub: skip syntax analysis and always succeed.
int yyparse(void) {
	if (g_parseRoot == nullptr) {
		g_parseRoot = buildFixedSemanticAstRoot();
	}
	return 0;
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

// Lexer stubs for link compatibility in parser-bypassed semantic tests.
FILE* yyin = nullptr;

int yylex(void) {
	return 0;
}

void lexerResetState() {}
const char* lexerLastLexeme() { return ""; }
const char* lexerLastType() { return ""; }
const char* lexerLastRule() { return ""; }
int lexerLastLine() { return 0; }
int lexerLastColumn() { return 0; }
void lexerClearErrors() {}
int lexerErrorCount() { return 0; }
const CompileError* lexerErrorAt(int) { return nullptr; }
int lexerAllocatedStringCount() { return 0; }
int lexerTokenCount() { return 0; }


void visitTree(ASTNode* node, int depth = 0) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }
    printf("%s", node->nodeType == NodeType::Program ? "Program" :
           node->nodeType == NodeType::Block ? "Block" :
           node->nodeType == NodeType::VarDecl ? "VarDecl" :
           node->nodeType == NodeType::ConstDecl ? "ConstDecl" :
           node->nodeType == NodeType::ProcDecl ? "ProcDecl" :
           node->nodeType == NodeType::FuncDecl ? "FuncDecl" :
           node->nodeType == NodeType::AssignStmt ? "AssignStmt" :
           node->nodeType == NodeType::IfStmt ? "IfStmt" :
           node->nodeType == NodeType::ForStmt ? "ForStmt" :
           node->nodeType == NodeType::CompoundStmt ? "CompoundStmt" :
           node->nodeType == NodeType::ProcCall ? "ProcCall" :
           node->nodeType == NodeType::BinaryExpr ? "BinaryExpr" :
           node->nodeType == NodeType::UnaryExpr ? "UnaryExpr" :
           node->nodeType == NodeType::Identifier ? "Identifier" :
           node->nodeType == NodeType::Literal ? "Literal" :
           node->nodeType == NodeType::ArrayAccess ? "ArrayAccess" :
           node->nodeType == NodeType::ArrayType ? "ArrayType" :
           node->nodeType == NodeType::ParamDecl ? "ParamDecl" :
           node->nodeType == NodeType::List ? "List" : "UnknownNode");
    if (auto* id = dynamic_cast<IdentifierNode*>(node)) {
        printf(" (%s)", id->identifier.c_str());
    }
    printf("\n");
    for (ASTNode* child : node->children) {
        visitTree(child, depth + 1);
    }
}

int main() {
    
    yyparse();
    ProgramNode* root = getParseResultRoot();
    if (root == nullptr) {
        fprintf(stderr, "Failed to build AST root\n");
        return 1;
    }
    visitTree(root);



    return 0;
}