#include <iostream>
#include <string>

#include "ast.h"
#include "error_handler.h"
#include "semantic_annotator.h"
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

bool testValidDeclarationAndAssignment() {
    ASTBuilder builder;
    ProgramNode* program = builder.makeProgram("t_valid");

    auto* typeInteger = builder.makeIdentifier("integer");
    auto* declX = builder.makeIdentifier("x");
    program->children.push_back(builder.makeVarDecl(declX, typeInteger));

    auto* lhs = builder.makeIdentifier("x");
    auto* rhs = builder.makeBinaryExpr("+", builder.makeLiteral("1"), builder.makeLiteral("2"));
    program->children.push_back(builder.makeAssignStmt(lhs, rhs));

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
    program->children.push_back(builder.makeAssignStmt(lhs, rhs));

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
    program->children.push_back(builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger1));
    program->children.push_back(builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger2));

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
    program->children.push_back(builder.makeVarDecl(builder.makeIdentifier("x"), typeInteger));

    auto* lhs = builder.makeIdentifier("x");
    auto* rhs = builder.makeLiteral("true");
    program->children.push_back(builder.makeAssignStmt(lhs, rhs));

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
    program->children.push_back(builder.makeProcDecl("p", paramList, nullptr));

    program->children.push_back(builder.makeVarDecl(builder.makeIdentifier("a"), builder.makeIdentifier("integer")));
    program->children.push_back(builder.makeVarDecl(builder.makeIdentifier("b"), builder.makeIdentifier("real")));
    return program;
}

bool testProcedureCallValid() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    program->children.push_back(builder.makeProcCall("p", {builder.makeIdentifier("a"), builder.makeIdentifier("b")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return !errors.hasErrors();
}

bool testProcedureCallArgCountMismatch() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    program->children.push_back(builder.makeProcCall("p", {builder.makeIdentifier("a")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Argument count mismatch in call to p");
}

bool testProcedureCallArgTypeMismatch() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    program->children.push_back(builder.makeProcCall("p", {builder.makeIdentifier("a"), builder.makeIdentifier("a")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "Argument type mismatch for parameter 2 in call to p");
}

bool testProcedureCallVarParamRequiresLValue() {
    ASTBuilder builder;
    ProgramNode* program = buildProgramWithProcedure(builder);

    program->children.push_back(builder.makeProcCall("p", {builder.makeLiteral("1"), builder.makeIdentifier("b")}));

    SymbolTable table;
    ErrorHandler errors;
    SemanticAnnotator annotator(table, errors);
    annotator.annotate(program);

    return containsMessage(errors, "var parameter requires assignable argument for parameter 1 in call to p");
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

    if (failed == 0) {
        std::cout << "[pass] semantic unit tests passed\n";
        return 0;
    }

    

    return 1;
}
