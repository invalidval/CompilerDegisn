#ifndef PASCC_SYMBOL_TABLE_H
#define PASCC_SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"

struct SymbolEntry {
    std::string name;
    DataType type;
    int scopeLevel;
    bool isConstant;
    bool isArray;
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool insert(const SymbolEntry& entry);
    const SymbolEntry* lookup(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes_;
};

#endif  // PASCC_SYMBOL_TABLE_H
