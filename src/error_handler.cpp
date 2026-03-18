#include "error_handler.h"

void ErrorHandler::report(int line, int column, const std::string& message) {
    errors_.push_back({line, column, message});
}

bool ErrorHandler::hasErrors() const {
    return !errors_.empty();
}

const std::vector<CompileError>& ErrorHandler::errors() const {
    return errors_;
}

void ErrorHandler::clear() {
    errors_.clear();
}
