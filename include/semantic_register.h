#ifndef SEMANTIC_REGISTER_H
#define SEMANTIC_REGISTER_H

#include "symbol_table.h"

namespace semantic_register {

inline void preregisterBuiltins(SymbolTable& symbolTable) {
	// read/write are language built-ins and should be available in global scope.
	(void)symbolTable.insert(SymbolEntry::makeProcedure("read"));
	(void)symbolTable.insert(SymbolEntry::makeProcedure("write"));
}

}  // namespace semantic_register

#endif  // SEMANTIC_REGISTER_H
