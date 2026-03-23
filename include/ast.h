#ifndef PASCC_AST_H
#define PASCC_AST_H

#include <memory>
#include <string>
#include <vector>

#include "common.h"
#include "symbol_table.h"

enum class NodeType {
    Program,
    Block,
    VarDecl,
    ConstDecl,
    ProcDecl,
    FuncDecl,
    AssignStmt,
    IfStmt,
    WhileStmt,
    ForStmt,
    CompoundStmt,
    ProcCall,
    BinaryExpr,
    UnaryExpr,
    Identifier,
    Literal,
    ArrayAccess,
    ArrayType,
    ParamDecl, 
    List
};

struct SourcePos {
    int line = 0;
    int col = 0;
};

enum class ListKind {
    // Pascal-S list mappings:
    // idlist -> Identifiers
    // statement_list -> Statements
    // expression_list / variable_list -> Expressions
    // parameter_list -> Parameters
    // const/var/subprogram declaration groups -> Declarations
    // period (multi-dimension ranges) -> ArrayRanges
    Unknown,
    Identifiers,
    Statements,
    Expressions,
    Parameters,
    Declarations,
    ArrayRanges
};

class ASTVisitor;

class ASTNode {
public:
    explicit ASTNode(NodeType type, SourcePos p = {});
    virtual ~ASTNode() = default;

    NodeType nodeType;
    DataType dataType = DataType::Unknown;
    SourcePos pos;
    std::vector<ASTNode*> children;
    const SymbolEntry* symbolEntry = nullptr;

    virtual void accept(ASTVisitor& visitor) = 0;
};

class ProgramNode : public ASTNode {
public:
    // programstruct -> program_head ';' program_body '.'
    // program_head -> program id | program id '(' idlist ')'
    explicit ProgramNode(const std::string& name, SourcePos p = {});

    std::string name;

    void accept(ASTVisitor& visitor) override;
};

class BlockNode : public ASTNode {
public:
    // program_body -> const_declarations var_declarations subprogram_declarations compound_statement
    // subprogram_body -> const_declarations var_declarations compound_statement
    explicit BlockNode(SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class VarDeclNode : public ASTNode {
public:
    // var_declaration -> idlist ':' type
    explicit VarDeclNode(ASTNode* idList, ASTNode* typeNode, SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class ConstDeclNode : public ASTNode {
public:
    // const_declaration -> id '=' const_value
    explicit ConstDeclNode(ASTNode* id, ASTNode* value, SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class ProcDeclNode : public ASTNode {
public:
    // subprogram_head -> procedure id formal_parameter
    explicit ProcDeclNode(const std::string& name, ASTNode* paramList, ASTNode* body, SourcePos p = {});
    std::string name;
    void accept(ASTVisitor& visitor) override;
};

class FuncDeclNode : public ASTNode {
public:
    // subprogram_head -> function id formal_parameter ':' basic_type
    explicit FuncDeclNode(const std::string& name, ASTNode* paramList, DataType retType, ASTNode* body, SourcePos p = {});
    std::string name;
    DataType retType; 
    void accept(ASTVisitor& visitor) override;
};

class AssignStmtNode : public ASTNode {
public:
    // statement -> variable assignop expression
    // statement -> func_id assignop expression
    explicit AssignStmtNode(ASTNode* lhs, ASTNode* rhs, SourcePos p = {});

    void accept(ASTVisitor& visitor) override;
};

class IfStmtNode : public ASTNode {
public:
    // statement -> if expression then statement else_part
    explicit IfStmtNode(ASTNode* cond, ASTNode* thenBranch, ASTNode* elseBranch, SourcePos p = {});

    void accept(ASTVisitor& visitor) override;
};

class WhileStmtNode : public ASTNode {
public:
    // statement -> while expression do statement
    explicit WhileStmtNode(ASTNode* cond, ASTNode* body, SourcePos p = {});

    void accept(ASTVisitor& visitor) override;
};

class ForStmtNode : public ASTNode {
public:
    // statement -> for id assignop expression to expression do statement
    // statement -> for id assignop expression downto expression do statement
    explicit ForStmtNode(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, bool isDownto = false, SourcePos p = {});

    bool isDownto = false;

    void accept(ASTVisitor& visitor) override;
};

class CompoundStmtNode : public ASTNode {
public:
    // compound_statement -> begin statement_list end
    explicit CompoundStmtNode(const std::vector<ASTNode*>& stmts, SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class ProcCallNode : public ASTNode {
public:
    // procedure_call -> id | id '(' expression_list ')'
    // factor -> id '(' expression_list ')'  (function-style call in expression)
    // statement -> read '(' variable_list ')' | write '(' expression_list ')'
    explicit ProcCallNode(const std::string& name, const std::vector<ASTNode*>& args, SourcePos p = {});

    std::string name;
    std::vector<bool> isVarParam;

    void accept(ASTVisitor& visitor) override;
};

class BinaryExprNode : public ASTNode {
public:
    // expression -> simple_expression relop simple_expression
    // simple_expression -> simple_expression addop term
    // term -> term mulop factor
    explicit BinaryExprNode(const std::string& op, ASTNode* lhs, ASTNode* rhs, SourcePos p = {});

    std::string op;

    void accept(ASTVisitor& visitor) override;
};

class UnaryExprNode : public ASTNode {
public:
    // factor -> not factor | uminus factor
    explicit UnaryExprNode(const std::string& op, ASTNode* expr, SourcePos p = {});
    std::string op;
    void accept(ASTVisitor& visitor) override;
};

class IdentifierNode : public ASTNode {
public:
    explicit IdentifierNode(const std::string& identifier, SourcePos p = {});

    std::string identifier;
    bool isLValue = false;

    void accept(ASTVisitor& visitor) override;
};

class LiteralNode : public ASTNode {
public:
    // factor -> num (and other scalar literal forms after lexical normalization)
    explicit LiteralNode(const std::string& value, SourcePos p = {});

    std::string value;

    void accept(ASTVisitor& visitor) override;
};


class ArrayAccessNode : public ASTNode {
public:
    // variable -> id id_varpart
    // id_varpart -> '[' expression_list ']'
    explicit ArrayAccessNode(ASTNode* base, ASTNode* index, int lowerBound = 0, SourcePos p = {});

    int lowerBound;

    void accept(ASTVisitor& visitor) override;
};

class ArrayTypeNode : public ASTNode {
public:
    // type -> array '[' period ']' of basic_type
    // Multi-dimensional arrays are represented by nesting ArrayTypeNode in elemType.
    ArrayTypeNode(ASTNode* lower, ASTNode* upper, ASTNode* elemType, SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class ParamDeclNode : public ASTNode {
public:
    // parameter -> var_parameter | value_parameter
    // value_parameter -> idlist ':' basic_type
    // var_parameter -> var value_parameter
    bool isVar; 
    ParamDeclNode(bool isVar, ASTNode* idList, ASTNode* typeNode, SourcePos p = {});
    void accept(ASTVisitor& visitor) override;
};

class ListNode : public ASTNode {
public:
    explicit ListNode(ListKind kind = ListKind::Unknown, SourcePos p = {});
    void add(ASTNode* node);

    ListKind kind;

    void accept(ASTVisitor& visitor) override;
};


class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(ConstDeclNode* node) = 0;
    virtual void visit(ProcDeclNode* node) = 0;
    virtual void visit(FuncDeclNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(ForStmtNode* node) = 0;
    virtual void visit(CompoundStmtNode* node) = 0;
    virtual void visit(ProcCallNode* node) = 0;
    virtual void visit(BinaryExprNode* node) = 0;
    virtual void visit(UnaryExprNode* node) = 0;
    virtual void visit(IdentifierNode* node) = 0;
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(ArrayAccessNode* node) = 0;
    virtual void visit(ArrayTypeNode* node) = 0;    
    virtual void visit(ParamDeclNode* node) = 0;    
    virtual void visit(ListNode* node) = 0;  
};

class ASTBuilder {
public:
    ASTBuilder() = default;

    ProgramNode* makeProgram(const std::string& name, SourcePos pos = {});
    BlockNode* makeBlock(SourcePos pos = {});
    VarDeclNode* makeVarDecl(ASTNode* idList, ASTNode* typeNode, SourcePos pos = {});
    ConstDeclNode* makeConstDecl(ASTNode* id, ASTNode* value, SourcePos pos = {});
    ProcDeclNode* makeProcDecl(const std::string& name, ASTNode* paramList, ASTNode* body, SourcePos pos = {});
    FuncDeclNode* makeFuncDecl(const std::string& name, ASTNode* paramList, DataType retType, ASTNode* body, SourcePos pos = {});
    AssignStmtNode* makeAssignStmt(ASTNode* lhs, ASTNode* rhs, SourcePos pos = {});
    IfStmtNode* makeIfStmt(ASTNode* cond, ASTNode* thenBranch, ASTNode* elseBranch, SourcePos pos = {});
    WhileStmtNode* makeWhileStmt(ASTNode* cond, ASTNode* body, SourcePos pos = {});
    ForStmtNode* makeForStmt(ASTNode* id, ASTNode* init, ASTNode* end, ASTNode* body, bool isDownto = false, SourcePos pos = {});
    CompoundStmtNode* makeCompoundStmt(const std::vector<ASTNode*>& stmts, SourcePos pos = {});
    ProcCallNode* makeProcCall(const std::string& name, const std::vector<ASTNode*>& args, SourcePos pos = {});
    BinaryExprNode* makeBinaryExpr(const std::string& op, ASTNode* lhs, ASTNode* rhs, SourcePos pos = {});
    UnaryExprNode* makeUnaryExpr(const std::string& op, ASTNode* expr, SourcePos pos = {});
    IdentifierNode* makeIdentifier(const std::string& identifier, SourcePos pos = {});
    LiteralNode* makeLiteral(const std::string& value, SourcePos pos = {});
    ArrayAccessNode* makeArrayAccess(ASTNode* base, ASTNode* index, int lowerBound = 0, SourcePos pos = {});
    ArrayTypeNode* makeArrayType(ASTNode* lower, ASTNode* upper, ASTNode* elemType, SourcePos pos = {});
    ParamDeclNode* makeParamDecl(bool isVar, ASTNode* idList, ASTNode* typeNode, SourcePos pos = {});
    ListNode* makeList(ListKind kind = ListKind::Unknown, SourcePos pos = {});

private:
    template <typename T, typename... Args>
    T* create(Args&&... args) {
        arena_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T*>(arena_.back().get());
    }

    std::vector<std::unique_ptr<ASTNode>> arena_;
};

#endif  // PASCC_AST_H
 