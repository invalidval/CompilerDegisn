#include "debug_utils.h"
#include "ast.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

namespace {

std::string toLowerCopy(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::string escapeJson(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 16);
    for (char ch : text) {
        switch (ch) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(ch); break;
        }
    }
    return out;
}

PasccLogLevel parseLogLevelEnv() {
    const char* raw = std::getenv("PASCC_LOG_LEVEL");
    if (raw == nullptr) {
        return PasccLogLevel::Off;
    }

    const std::string text = toLowerCopy(std::string(raw));
    if (text.empty() || text == "off" || text == "0") {
        return PasccLogLevel::Off;
    }
    if (text == "error" || text == "1") {
        return PasccLogLevel::Error;
    }
    if (text == "warn" || text == "warning" || text == "2") {
        return PasccLogLevel::Warn;
    }
    if (text == "info" || text == "3") {
        return PasccLogLevel::Info;
    }
    if (text == "debug" || text == "4") {
        return PasccLogLevel::Debug;
    }
    return PasccLogLevel::Off;
}

PasccLogLevel configuredLogLevel() {
    static const PasccLogLevel level = parseLogLevelEnv();
    return level;
}

const char* nodeTypeName(NodeType type) {
    return type == NodeType::Program ? "Program" :
           type == NodeType::Block ? "Block" :
           type == NodeType::VarDecl ? "VarDecl" :
           type == NodeType::ConstDecl ? "ConstDecl" :
           type == NodeType::ProcDecl ? "ProcDecl" :
           type == NodeType::FuncDecl ? "FuncDecl" :
           type == NodeType::AssignStmt ? "AssignStmt" :
           type == NodeType::IfStmt ? "IfStmt" :
           type == NodeType::WhileStmt ? "WhileStmt" :
           type == NodeType::ForStmt ? "ForStmt" :
           type == NodeType::BreakStmt ? "BreakStmt" :
           type == NodeType::CompoundStmt ? "CompoundStmt" :
           type == NodeType::ProcCall ? "ProcCall" :
           type == NodeType::BinaryExpr ? "BinaryExpr" :
           type == NodeType::UnaryExpr ? "UnaryExpr" :
           type == NodeType::Identifier ? "Identifier" :
           type == NodeType::Literal ? "Literal" :
           type == NodeType::ArrayAccess ? "ArrayAccess" :
           type == NodeType::ArrayType ? "ArrayType" :
           type == NodeType::ParamDecl ? "ParamDecl" :
           type == NodeType::List ? "List" : "UnknownNode";
}

const char* dataTypeName(DataType type) {
    return type == DataType::Integer ? "integer" :
           type == DataType::Real ? "real" :
           type == DataType::Boolean ? "boolean" :
           type == DataType::Char ? "char" :
           type == DataType::Procedure ? "procedure" :
           type == DataType::Function ? "function" : "unknown";
}

const char* symbolKindName(SymbolKind kind) {
    return kind == SymbolKind::Variable ? "variable" :
           kind == SymbolKind::Constant ? "constant" :
           kind == SymbolKind::Procedure ? "procedure" :
           kind == SymbolKind::Function ? "function" : "parameter";
}

void printSymbolEntryBrief(const SymbolEntry* entry) {
    if (entry == nullptr) {
        return;
    }

    std::cout << " [sym: {name=" << entry->name
              << ", kind=" << symbolKindName(entry->kind)
              << ", scope=" << entry->scopeLevel
              << ", type=" << dataTypeName(entry->type)
              << ", isArray=" << (entry->isArray ? "true" : "false")
              << ", isVarParam=" << (entry->isVarParam ? "true" : "false")
              << "}]";
    if (entry->isConstantLike()) {
        std::cout << " [constLiteral=" << entry->constLiteralText << "]";
    }
    if (entry->isArray) {
        std::cout << " [arrayBounds=";
        for (const auto& bound : entry->arrayBounds) {
            std::cout << "[" << bound.lower << ".." << bound.upper << "]";
        }
        std::cout << "]";
    }
}

}  // namespace

const char* pasccLogLevelName(PasccLogLevel level) {
    switch (level) {
    case PasccLogLevel::Error: return "ERROR";
    case PasccLogLevel::Warn: return "WARN";
    case PasccLogLevel::Info: return "INFO";
    case PasccLogLevel::Debug: return "DEBUG";
    case PasccLogLevel::Off: return "OFF";
    }
    return "OFF";
}

bool pasccLogEnabled(PasccLogLevel level) {
    const PasccLogLevel configured = configuredLogLevel();
    if (configured == PasccLogLevel::Off || level == PasccLogLevel::Off) {
        return false;
    }
    return static_cast<int>(level) <= static_cast<int>(configured);
}

void pasccLog(const std::string& stage, PasccLogLevel level, const std::string& message) {
    if (!pasccLogEnabled(level)) {
        return;
    }

    std::cerr << "[PASCC_LOG]{\"stage\":\"" << escapeJson(stage)
              << "\",\"level\":\"" << escapeJson(pasccLogLevelName(level))
              << "\",\"message\":\"" << escapeJson(message)
              << "\"}" << "\n";
}

void printAstNode(const ASTNode *node, int indent) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    std::cout << nodeTypeName(node->nodeType);
    if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        std::cout << " (" << id->identifier << ")";
    }
    std::cout << std::endl;
    for (ASTNode* child : node->children) {
        printAstNode(child, indent + 1);
    }
}
void printAnnotatedAstNode(const ASTNode *node, int indent) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    std::cout << nodeTypeName(node->nodeType);
    if (auto* id = dynamic_cast<const IdentifierNode*>(node)) {
        std::cout << " (" << id->identifier
                  << ", type: " << dataTypeName(id->dataType)
                  << ", isLValue: " << (id->isLValue ? "true" : "false") << ")";
    }
    std::cout << " (type: " << dataTypeName(node->dataType) << ")";
    printSymbolEntryBrief(node->symbolEntry);

    std::cout << std::endl;
    for (ASTNode* child : node->children) {
        printAnnotatedAstNode(child, indent + 1);
    }
}