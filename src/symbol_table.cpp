#include "symbol_table.h"

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

bool SymbolTable::insert(const SymbolEntry& entry) {
    auto& current = scopes_.back();
    if (current.find(entry.name) != current.end()) {
        return false;
    }
    current[entry.name] = entry;
    return true;
}

const SymbolEntry* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}
