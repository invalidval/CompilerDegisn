#include "codegen_utils.h"

std::string CodegenUtils::wrapAsCProgram(const std::string& body) {
    return "#include <stdio.h>\n\nint main(void) {\n" + body + "\n    return 0;\n}\n";
}
