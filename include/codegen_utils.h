#ifndef PASCC_CODEGEN_UTILS_H
#define PASCC_CODEGEN_UTILS_H

#include <string>

class CodegenUtils {
public:
    static std::string wrapAsCProgram(const std::string& body);
};

#endif  // PASCC_CODEGEN_UTILS_H
