#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc == 1 || (argc > 1 && std::string(argv[1]) == "--help")) {
        std::cout << "pascc - Pascal-S to C compiler (project skeleton)\n";
        std::cout << "Usage: pascc -i <input.pas> [-o output.c]\n";
        return 0;
    }

    std::cout << "Compiler skeleton is ready. Core pipeline implementation is pending.\n";
    return 0;
}
