#include "symbol_table.h"

#include <cctype>

SymbolEntry SymbolEntry::makeVariable(const std::string& name, DataType type, bool isArray) {
    SymbolEntry entry;
    entry.name = name;
    entry.kind = SymbolKind::Variable;
    entry.type = type;
    entry.isArray = isArray;
    return entry;
}

SymbolEntry SymbolEntry::makeConstant(const std::string& name, DataType type, const std::string& literalText) {
    SymbolEntry entry;
    entry.name = name;
    entry.kind = SymbolKind::Constant;
    entry.type = type;
    entry.isArray = false;
    if (!literalText.empty()) {
        entry.hasConstLiteral = true;
        entry.constLiteralText = literalText;
    }
    return entry;
}

SymbolEntry SymbolEntry::makeProcedure(const std::string& name) {
    SymbolEntry entry;
    entry.name = name;
    entry.kind = SymbolKind::Procedure;
    entry.type = DataType::Procedure;
    entry.isArray = false;
    return entry;
}

SymbolEntry SymbolEntry::makeFunction(const std::string& name, DataType returnType) {
    SymbolEntry entry;
    entry.name = name;
    entry.kind = SymbolKind::Function;
    entry.type = returnType;
    entry.isArray = false;
    return entry;
}

SymbolEntry SymbolEntry::makeParameter(const std::string& name, DataType type, bool isVarParam) {
    SymbolEntry entry;
    entry.name = name;
    entry.kind = SymbolKind::Parameter;
    entry.type = type;
    entry.isArray = false;
    entry.isVarParam = isVarParam;
    return entry;
}

std::string SymbolTable::normalizeName(const std::string& name) {
    std::string normalized = name;
    for (char& ch : normalized) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return normalized;
}

SymbolTable::SymbolTable() {
    scopes_.emplace_back();
}

void SymbolTable::enterScope() {
    scopes_.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

bool SymbolTable::insert(SymbolEntry entry) {
    entry.name = normalizeName(entry.name);
    auto& current = scopes_.back();
    if (current.find(entry.name) != current.end()) {
        return false;
    }
    entry.scopeLevel = currentScopeLevel();

    // Keep SymbolEntry addresses stable for AST annotations even after scope pop.
    entryArena_.push_back(std::make_unique<SymbolEntry>(std::move(entry)));
    const SymbolEntry* stored = entryArena_.back().get();
    current[stored->name] = stored;
    return true;
}

int SymbolTable::currentScopeLevel() const {
    return static_cast<int>(scopes_.size()) - 1;
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    const std::string normalized = normalizeName(name);
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(normalized);
        if (found != it->end()) {
            return found->second;
        }
    }
    return nullptr;
}

const SymbolEntry* SymbolTable::lookupCurrentScope(const std::string& name) const {
    if (scopes_.empty()) {
        return nullptr;
    }
    const auto& current = scopes_.back();
    auto found = current.find(normalizeName(name));
    if (found == current.end()) {
        return nullptr;
    }
    return found->second;
}
