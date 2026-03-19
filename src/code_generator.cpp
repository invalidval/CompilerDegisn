#include "code_generator.h"

#include <sstream>

#include "codegen_utils.h"

std::string CodeGenerator::generate(ProgramNode* root) {
    body_.clear();
    currentExpr_.clear();

    if (root != nullptr) {
        root->accept(*this);
    }

    if (body_.empty()) {
        body_ = "    /* TODO: emit statements from AST */\n";
    }

    return CodegenUtils::wrapAsCProgram(body_);
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

void CodeGenerator::visit(AssignStmtNode* node) {
    std::string lhs = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string rhs = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);
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

void CodeGenerator::visit(ArrayAccessNode* node) {
    std::string base = emitNode(node->children.size() > 0 ? node->children[0] : nullptr);
    std::string index = emitNode(node->children.size() > 1 ? node->children[1] : nullptr);

    if (node->lowerBound == 0) {
        currentExpr_ = base + "[" + index + "]";
    } else {
        currentExpr_ = base + "[(" + index + ") - " + std::to_string(node->lowerBound) + "]";
    }
}

void CodeGenerator::visit(ProcCallNode* node) {
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
