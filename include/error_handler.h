#ifndef PASCC_ERROR_HANDLER_H
#define PASCC_ERROR_HANDLER_H

#include <string>
#include <vector>

struct CompileError {
    int line;
    int column;
    std::string message;
};

class ErrorHandler {
public:
    void report(int line, int column, const std::string& message);
    bool hasErrors() const;
    const std::vector<CompileError>& errors() const;
    void clear();

private:
    std::vector<CompileError> errors_;
};

#endif  // PASCC_ERROR_HANDLER_H
