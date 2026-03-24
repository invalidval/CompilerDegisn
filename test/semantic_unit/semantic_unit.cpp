#include <iostream>
#include <string>

#include "ast.h"
#include "error_handler.h"
#include "semantic_annotator.h"
#include "semantic_register.h"
#include "symbol_table.h"

namespace {

bool containsMessage(const ErrorHandler& handler, const std::string& text) {
    for (const auto& err : handler.errors()) {
        if (err.message.find(text) != std::string::npos) {
            return true;
        }
    }
    return false;
}

BlockNode* ensureProgramBlock(ASTBuilder& builder, ProgramNode* program) {
    if (!program->children.empty() && program->children[0] != nullptr &&
        program->children[0]->nodeType == NodeType::Block) {
        return static_cast<BlockNode*>(program->children[0]);
    }

    auto* block = builder.makeBlock();
    program->children.insert(program->children.begin(), block);
    return block;
}

void addToProgramBody(ASTBuilder& builder, ProgramNode* program, ASTNode* node) {
    ensureProgramBlock(builder, program)->children.push_back(node);
}

bool testValidDeclarationAndAssignment() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_valid");

    auto* typeInteger = builder.makeIdentifier("integer");
    auto* declX = builder.makeIdentifier("x");
    addToProgramBody(builder, program, builder.makeVarDecl(declX, typeInteger));

    auto* lhs = builder.makeIdentifier("x");
    auto* rhs = builder.makeBinaryExpr("+", builder.makeLiteral("1"), builder.makeLiteral("2"));
    addToProgramBody(builder, program, builder.makeAssignStmt(lhs, rhs));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !errors.hasErrors();
}

bool testUndefinedIdentifier() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_undef");

    auto* lhs = builder.makeIdentifier("x");
    auto* rhs = builder.makeLiteral("1");
    addToProgramBody(builder, program, builder.makeAssignStmt(lhs, rhs));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);
    return containsMessage(errors, "Undefined identifier: x");
}

bool testRedefinition() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_redef");

    auto* typeInteger1 = builder.makeIdentifier("integer");
    auto* typeInteger2 = builder.makeIdentifier("integer");
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger1));
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger2));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Redefinition of identifier: x");
}

bool testAssignmentTypeMismatch() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_type");

    auto* typeInteger = builder.makeIdentifier("integer");
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger));

    auto* lhs = builder.makeIdentifier("x");
    auto* rhs = builder.makeLiteral("true");
    addToProgramBody(builder, program, builder.makeAssignStmt(lhs, rhs));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Type mismatch in assignment");
}

ProgramNode* buildProgramWithProcedure(ASTBuilder& builder) {
    ProgramNode* program = builder.makeProgram("t_proc");

    auto* paramList = builder.makeList(ListKind::Parameters);
    paramList->add(builder.makeParamDecl(true, builder.makeIdentifier("x"), builder.makeIdentifier("integer")));
    paramList->add(builder.makeParamDecl(false, builder.makeIdentifier("y"), builder.makeIdentifier("real")));
    addToProgramBody(builder, program, builder.makeProcDecl("p", paramList, nullptr));

    addToProgramBody(builder, program,
                     builder.makeVarDecl(builder.makeIdentifier("a"), builder.makeIdentifier("integer")));
    addToProgramBody(builder, program,
                     builder.makeVarDecl(builder.makeIdentifier("b"), builder.makeIdentifier("real")));
    return program;
}

bool testProcedureCallValid() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    addToProgramBody(builder, program,
                     builder.makeProcCall("p", {builder.makeIdentifier("a"), builder.makeIdentifier("b")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !errors.hasErrors();
}

bool testProcedureCallArgCountMismatch() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    addToProgramBody(builder, program, builder.makeProcCall("p", {builder.makeIdentifier("a")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Argument count mismatch in call to p");
}

bool testProcedureCallArgTypeMismatch() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    addToProgramBody(builder, program,
                     builder.makeProcCall("p", {builder.makeIdentifier("a"), builder.makeIdentifier("a")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Argument type mismatch for parameter 2 in call to p");
}

bool testProcedureCallVarParamRequiresLValue() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    addToProgramBody(builder, program,
                     builder.makeProcCall("p", {builder.makeLiteral("1"), builder.makeIdentifier("b")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "var parameter requires assignable argument for parameter 1 in call to p");
}

bool testFunctionResultAssignmentValidInsideFunction() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_func_ret_ok");

    ASTNode* body = builder.makeAssignStmt(builder.makeIdentifier("f"), builder.makeLiteral("1"));
    addToProgramBody(builder, program, builder.makeFuncDecl("f", nullptr, DataType::Integer, body));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !errors.hasErrors();
}

bool testFunctionResultAssignmentInvalidOutsideFunction() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_func_ret_bad");

    addToProgramBody(builder, program, builder.makeFuncDecl("f", nullptr, DataType::Integer, nullptr));
    addToProgramBody(builder, program,
                     builder.makeAssignStmt(builder.makeIdentifier("f"), builder.makeLiteral("1")));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Left-hand side of assignment is not assignable");
}

bool testArrayIndexOutOfBounds() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_arr_bound");

    ASTNode* arrType = builder.makeArrayType(
        builder.makeLiteral("1"),
        builder.makeLiteral("3"),
        builder.makeIdentifier("integer")
    );
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("arr"), arrType));

    ASTNode* badAccess = builder.makeArrayAccess(builder.makeIdentifier("arr"), builder.makeLiteral("5"));
    addToProgramBody(builder, program, builder.makeAssignStmt(badAccess, builder.makeLiteral("1")));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Array index out of bounds");
}

bool testArrayIndexInBounds() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_arr_ok");

    ASTNode* arrType = builder.makeArrayType(
        builder.makeLiteral("1"),
        builder.makeLiteral("3"),
        builder.makeIdentifier("integer")
    );
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("arr"), arrType));

    ASTNode* okAccess = builder.makeArrayAccess(builder.makeIdentifier("arr"), builder.makeLiteral("2"));
    addToProgramBody(builder, program, builder.makeAssignStmt(okAccess, builder.makeLiteral("1")));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !errors.hasErrors();
}

bool testProcedureCallCannotBeUsedAsValue() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    addToProgramBody(builder, program,
        builder.makeAssignStmt(
            builder.makeIdentifier("a"),
            builder.makeProcCall("p", {builder.makeIdentifier("a"), builder.makeIdentifier("b")})
        )
    );

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Procedure call cannot be used as a value: p");
}

bool testArrayIndexConstExpressionOutOfBounds() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_arr_expr_oob");

    ASTNode* arrType = builder.makeArrayType(
        builder.makeLiteral("1"),
        builder.makeLiteral("3"),
        builder.makeIdentifier("integer")
    );
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("arr"), arrType));

    ASTNode* exprIndex = builder.makeBinaryExpr("+", builder.makeLiteral("1"), builder.makeLiteral("3"));
    ASTNode* badAccess = builder.makeArrayAccess(builder.makeIdentifier("arr"), exprIndex);
    addToProgramBody(builder, program, builder.makeAssignStmt(badAccess, builder.makeLiteral("1")));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Array index out of bounds");
}

bool testMultiDimArrayBoundsCheck() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_arr_2d");

    ASTNode* inner = builder.makeArrayType(
        builder.makeLiteral("10"),
        builder.makeLiteral("12"),
        builder.makeIdentifier("integer")
    );
    ASTNode* outer = builder.makeArrayType(
        builder.makeLiteral("1"),
        builder.makeLiteral("2"),
        inner
    );
    addToProgramBody(builder, program, builder.makeVarDecl(builder.makeIdentifier("m"), outer));

    ASTNode* first = builder.makeArrayAccess(builder.makeIdentifier("m"), builder.makeLiteral("2"));
    ASTNode* secondBad = builder.makeArrayAccess(first, builder.makeLiteral("99"));
    addToProgramBody(builder, program, builder.makeAssignStmt(secondBad, builder.makeLiteral("1")));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Array index out of bounds");
}

bool testBuiltinReadWritePreregistered() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_builtin_rw");

    addToProgramBody(builder, program,
                     builder.makeVarDecl(builder.makeIdentifier("x"), builder.makeIdentifier("integer")));
    addToProgramBody(builder, program, builder.makeProcCall("read", {builder.makeIdentifier("x")}));
    addToProgramBody(builder, program, builder.makeProcCall("write", {builder.makeIdentifier("x")}));

    SymbolTable table;
    semantic_register::preregisterBuiltins(table);
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !containsMessage(errors, "Undefined procedure/function: read") &&
           !containsMessage(errors, "Undefined procedure/function: write");
}

}  // namespace

int main() {
    int failed = 0;

    if (!testValidDeclarationAndAssignment()) {
        std::cerr << "[fail] valid declaration/assignment should pass\n";
        ++failed;
    }
    if (!testUndefinedIdentifier()) {
        std::cerr << "[fail] undefined identifier check did not trigger\n";
        ++failed;
    }
    if (!testRedefinition()) {
        std::cerr << "[fail] redefinition check did not trigger\n";
        ++failed;
    }
    if (!testAssignmentTypeMismatch()) {
        std::cerr << "[fail] assignment type mismatch check did not trigger\n";
        ++failed;
    }
    if (!testProcedureCallValid()) {
        std::cerr << "[fail] valid procedure call should pass\n";
        ++failed;
    }
    if (!testProcedureCallArgCountMismatch()) {
        std::cerr << "[fail] procedure call arg-count mismatch did not trigger\n";
        ++failed;
    }
    if (!testProcedureCallArgTypeMismatch()) {
        std::cerr << "[fail] procedure call arg-type mismatch did not trigger\n";
        ++failed;
    }
    if (!testProcedureCallVarParamRequiresLValue()) {
        std::cerr << "[fail] procedure call var-parameter lvalue check did not trigger\n";
        ++failed;
    }
    if (!testFunctionResultAssignmentValidInsideFunction()) {
        std::cerr << "[fail] function result assignment inside function should pass\n";
        ++failed;
    }
    if (!testFunctionResultAssignmentInvalidOutsideFunction()) {
        std::cerr << "[fail] function result assignment outside function did not trigger\n";
        ++failed;
    }
    if (!testArrayIndexOutOfBounds()) {
        std::cerr << "[fail] array out-of-bounds check did not trigger\n";
        ++failed;
    }
    if (!testArrayIndexInBounds()) {
        std::cerr << "[fail] in-bounds array access should pass\n";
        ++failed;
    }
    if (!testProcedureCallCannotBeUsedAsValue()) {
        std::cerr << "[fail] procedure call used as value did not trigger\n";
        ++failed;
    }
    if (!testArrayIndexConstExpressionOutOfBounds()) {
        std::cerr << "[fail] array index const-expression out-of-bounds did not trigger\n";
        ++failed;
    }
    if (!testMultiDimArrayBoundsCheck()) {
        std::cerr << "[fail] multi-dimensional array bound check did not trigger\n";
        ++failed;
    }
    if (!testBuiltinReadWritePreregistered()) {
        std::cerr << "[fail] builtin read/write preregistration did not work\n";
        ++failed;
    }

    if (failed == 0) {
        std::cout << "[pass] semantic unit tests passed\n";
        return 0;
    }

    

    return 1;
}
