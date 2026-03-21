#ifndef PASCC_SYMBOL_TABLE_H
#define PASCC_SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"

struct SymbolEntry {
    // 符号表条目
    std::string name; // 名字
    DataType type; // 类型
    int scopeLevel; // 作用域等级（也就是scope_项目的索引，from 0）
    bool isConstant; // 标识位（非常易于扩展）
    bool isArray;
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();
    bool insert(const SymbolEntry& entry);
    
    // 全局查找，可根据scopeLevel来判断是否属于当前作用域
    const SymbolEntry* lookup(const std::string& name) const;

private:
    // 栈式哈基符号表
    std::vector<std::unordered_map<std::string, SymbolEntry>> scopes_;
};

#endif  // PASCC_SYMBOL_TABLE_H
