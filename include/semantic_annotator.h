#ifndef PASCC_SEMANTIC_ANNOTATOR_H
#define PASCC_SEMANTIC_ANNOTATOR_H

#include "ast.h"
#include "error_handler.h"
#include "symbol_table.h"

class SemanticAnnotator {
public:
    SemanticAnnotator(SymbolTable& symbolTable, ErrorHandler& errorHandler);

    void annotate(ASTNode* root);

private:
    void annotateNode(ASTNode* node);
    void annotateIdentifier(IdentifierNode* node);

    SymbolTable& symbolTable_;
    ErrorHandler& errorHandler_;
};

#endif  // PASCC_SEMANTIC_ANNOTATOR_H
