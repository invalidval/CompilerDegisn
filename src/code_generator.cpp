#include "code_generator.h"
#include "codegen_utils.h"
#include "symbol_table.h" // 新增：引入符号表类型定义

#include <sstream>

std::string CodeGenerator::generate(ProgramNode* root) {
    reset();
    if (root != nullptr) {
        root->accept(*this);
    }
    if (body_.empty()) {
        body_ = "    /* TODO: emit statements from AST */\n";
    }
    return CodegenUtils::wrapAsCProgram(body_);
}

void CodeGenerator::reset() {
    body_.clear();
    currentExpr_.clear();
}

void CodeGenerator::visit(ProgramNode* node) {
    for (ASTNode* child : node->children) {
        std::string line = emitNode(child);
        if (!line.empty()) {
            body_ += "    " + line;
            if (line.back() != '\n') {
                body_ += "\n";
            }
        }
    }
}

void CodeGenerator::visit(BlockNode* node) {
    std::ostringstream oss;
    for (ASTNode* child : node->children) {
        std::string part = emitNode(child);
        if (!part.empty()) {
            oss << part;
            if (part.back() != '\n') {
                oss << "\n";
            }
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(VarDeclNode* node) {
    // 生成变量声明代码
    // 示例：int a, b;
    std::string decl = CodegenUtils::emitVarDecl(node);
    if (!decl.empty()) {
        body_ += "    " + decl + "\n";
    }
    currentExpr_.clear();
}

void CodeGenerator::visit(ConstDeclNode* node) {
    // 生成常量声明代码
    // 示例：const int N = 10;
    std::string decl = CodegenUtils::emitConstDecl(node);
    if (!decl.empty()) {
        body_ += "    " + decl + "\n";
    }
    currentExpr_.clear();
}

void CodeGenerator::visit(ProcDeclNode* node) {
    // 生成过程声明代码
    std::string procCode = CodegenUtils::emitProcDecl(node, *this);
    if (!procCode.empty()) {
        body_ += procCode + "\n";
    }
    currentExpr_.clear();
}

void CodeGenerator::visit(FuncDeclNode* node) {
    // 生成函数声明代码
    std::string funcCode = CodegenUtils::emitFuncDecl(node, *this);
    if (!funcCode.empty()) {
        body_ += funcCode + "\n";
    }
    currentExpr_.clear();
}

void CodeGenerator::visit(IdentifierNode* node) {
    currentExpr_ = node->identifier;
}

void CodeGenerator::visit(LiteralNode* node) {
    currentExpr_ = node->value;
}

void CodeGenerator::visit(BinaryExprNode* node) {
    std::string lhs = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string rhs = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    currentExpr_ = "(" + lhs + " " + node->op + " " + rhs + ")";
}

void CodeGenerator::visit(UnaryExprNode* node) {
    std::string operand = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    if (node->op == "not") {
        currentExpr_ = "(!" + operand + ")";
    } else {
        currentExpr_ = "(" + node->op + operand + ")";
    }
}

void CodeGenerator::visit(AssignStmtNode* node) {
    // 判断是否为函数返回值赋值
    IdentifierNode* idNode = dynamic_cast<IdentifierNode*>(node->children[0]);
    if (idNode && idNode->symbolEntry) {
        // 正确判断：符号种类为函数
        if (idNode->symbolEntry->kind == SymbolKind::Function) {
            std::string rhs = emitNode(node->children[1]);
            currentExpr_ = "_retval = " + rhs + ";";
            return;
        }
    }
    // 普通赋值
    std::string lhs = emitNode(node->children[0]);
    std::string rhs = emitNode(node->children[1]);
    currentExpr_ = lhs + " = " + rhs + ";";
}

void CodeGenerator::visit(IfStmtNode* node) {
    std::string cond = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string thenStmt = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);

    std::ostringstream oss;
    oss << "if (" << cond << ") {\n";
    if (!thenStmt.empty()) {
        oss << "        " << thenStmt;
        if (thenStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "    }";

    if (node->children.size() > 2) {
        std::string elseStmt = emitNode(node->children[2]);
        oss << " else {\n";
        if (!elseStmt.empty()) {
            oss << "        " << elseStmt;
            if (elseStmt.back() != '\n') {
                oss << "\n";
            }
        }
        oss << "    }";
    }

    currentExpr_ = oss.str();
}

void CodeGenerator::visit(WhileStmtNode* node) {
    std::string cond = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string bodyStmt = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);

    std::ostringstream oss;
    oss << "while (" << cond << ") {\n";
    if (!bodyStmt.empty()) {
        oss << "        " << bodyStmt;
        if (bodyStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "    }";
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ForStmtNode* node) {
    std::string iter = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string init = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
    std::string end = emitNode(node->children.size() > 2 ? node->children[2] : nullptr);
    std::string bodyStmt = emitNode(node->children.size() > 3 ? node->children[3] : nullptr);

    std::string cmp = node->isDownto ? ">=" : "<=";
    std::string step = node->isDownto ? "--" : "++";

    std::ostringstream oss;
    oss << "for (" << iter << " = " << init << "; " << iter << " " << cmp << " " << end
        << "; " << iter << step << ") {\n";
    if (!bodyStmt.empty()) {
        oss << "        " << bodyStmt;
        if (bodyStmt.back() != '\n') {
            oss << "\n";
        }
    }
    oss << "    }";
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(CompoundStmtNode* node) {
    std::ostringstream oss;
    for (ASTNode* stmt : node->children) {
        std::string line = emitNode(stmt);
        if (!line.empty()) {
            oss << line;
            if (line.back() != '\n') {
                oss << "\n";
            }
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    std::string base = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string index = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);

    if (node->lowerBound == 0) {
        currentExpr_ = base + "[" + index + "]";
    } else {
        currentExpr_ = base + "[(" + index + ") - " + std::to_string(node->lowerBound) + "]";
    }
}

void CodeGenerator::visit(ArrayTypeNode* /*node*/) {
    currentExpr_.clear();
}

void CodeGenerator::visit(ParamDeclNode* /*node*/) {
    currentExpr_.clear();
}

void CodeGenerator::visit(ListNode* node) {
    std::ostringstream oss;
    for (ASTNode* item : node->children) {
        std::string piece = emitNode(item);
        if (!piece.empty()) {
            if (oss.tellp() > 0) {
                oss << ", ";
            }
            oss << piece;
        }
    }
    currentExpr_ = oss.str();
}

void CodeGenerator::visit(ProcCallNode* node) {
    // 特殊处理 read/write
    if (node->name == "read") {
        std::string code = CodegenUtils::emitReadStmt(node);
        currentExpr_ = code;
        return;
    }
    if (node->name == "write") {
        std::string code = CodegenUtils::emitWriteStmt(node);
        currentExpr_ = code;
        return;
    }
    // 普通过程调用
    std::ostringstream oss;
    oss << node->name << "(";

    for (size_t i = 0; i < node->children.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }

        std::string arg = emitNode(node->children[i]);
        if (i < node->isVarParam.size() && node->isVarParam[i]) {
            oss << "&" << arg;
        } else {
            oss << arg;
        }
    }

    oss << ")";
    currentExpr_ = oss.str();
}

std::string CodeGenerator::emitNode(ASTNode* node) {
    if (node == nullptr) {
        return "";
    }

    node->accept(*this);
    return currentExpr_;
}
