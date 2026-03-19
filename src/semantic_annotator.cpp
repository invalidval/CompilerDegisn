#include "semantic_annotator.h"

SemanticAnnotator::SemanticAnnotator(SymbolTable& symbolTable, ErrorHandler& errorHandler)
    : symbolTable_(symbolTable), errorHandler_(errorHandler) {}

void SemanticAnnotator::annotate(ASTNode* root) {
    annotateNode(root);
}

void SemanticAnnotator::annotateNode(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    if (auto* identifier = dynamic_cast<IdentifierNode*>(node)) {
        annotateIdentifier(identifier);
    }

    for (ASTNode* child : node->children) {
        annotateNode(child);
    }
}

void SemanticAnnotator::annotateIdentifier(IdentifierNode* node) {
    const SymbolEntry* entry = symbolTable_.lookup(node->identifier);
    if (entry == nullptr) {
        errorHandler_.report(node->pos.line, node->pos.col,
            "Undefined identifier: " + node->identifier);
        node->dataType = DataType::Unknown;
        return;
    }

    node->symbolEntry = entry;
    node->dataType = entry->type;
    node->isLValue = !entry->isConstant;
}
