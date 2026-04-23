#ifndef PASCC_SYMBOL_TABLE_H
#define PASCC_SYMBOL_TABLE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "common.h"

enum class SymbolKind {
    Variable,
    Constant,
    Procedure,
    Function,
    Parameter,
    TypeAlias
};

struct ArrayBound {
    int lower = 0;
    int upper = -1;
};

struct ParamInfo {
    std::string name;
    DataType type = DataType::Unknown;
    bool isVarParam = false;
};

struct SymbolEntry {
    std::string name;
    SymbolKind kind = SymbolKind::Variable;
    DataType type = DataType::Unknown;
    int scopeLevel = 0;

    bool isArray = false;
    std::vector<ArrayBound> arrayBounds;

    bool hasConstLiteral = false;
    std::string constLiteralText;
    bool isStringLikeConst = false;

    std::vector<ParamInfo> params;
    bool isVarParam = false;

    // For record types: store field information
    std::vector<ParamInfo> fields;

    // For variables of user-defined types: reference to the type name
    std::string typeName;

    bool isConstantLike() const {
        return kind == SymbolKind::Constant;
    }

    static SymbolEntry makeVariable(const std::string& name, DataType type, bool isArray = false);
    static SymbolEntry makeConstant(const std::string& name, DataType type, const std::string& literalText = "");
    static SymbolEntry makeProcedure(const std::string& name);
    static SymbolEntry makeFunction(const std::string& name, DataType returnType);
    static SymbolEntry makeParameter(const std::string& name, DataType type, bool isVarParam);
    static SymbolEntry makeTypeAlias(const std::string& name, DataType type);
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool insert(SymbolEntry entry);

    int currentScopeLevel() const;

    const SymbolEntry* lookup(const std::string& name) const;
    const SymbolEntry* lookupCurrentScope(const std::string& name) const;

private:
    static std::string normalizeName(const std::string& name);
    std::vector<std::unordered_map<std::string, const SymbolEntry*>> scopes_;
    std::vector<std::unique_ptr<SymbolEntry>> entryArena_;
};

#endif  // PASCC_SYMBOL_TABLE_H
