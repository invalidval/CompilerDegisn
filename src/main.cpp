#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "code_generator.h"
#include "error_handler.h"
#include "parser_bridge.h"
#include "semantic_annotator.h"
#include "symbol_table.h"

extern FILE* yyin;
int yyparse(void);

namespace {

void printUsage() {
    std::cout << "pascc - Pascal-S to C compiler (project skeleton)\n";
    std::cout << "Usage: pascc -i <input.pas> [-o output.c]\n";
}

bool parseArgs(int argc, char* argv[], std::string& inputPath, std::string& outputPath,
    bool& shouldExit, int& exitCode) {
    shouldExit = false;
    exitCode = 0;
    outputPath = "out.c";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printUsage();
            shouldExit = true;
            exitCode = 0;
            return true;
        }
        if (arg == "-i" && i + 1 < argc) {
            inputPath = argv[++i];
            continue;
        }
        if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
            continue;
        }
    }

    if (inputPath.empty()) {
        printUsage();
        shouldExit = true;
        exitCode = 1;
        return true;
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string inputPath;
    std::string outputPath;
    bool shouldExit = false;
    int exitCode = 0;
    if (!parseArgs(argc, argv, inputPath, outputPath, shouldExit, exitCode)) {
        return 1;
    }
    if (shouldExit) {
        return exitCode;
    }

    FILE* input = std::fopen(inputPath.c_str(), "r");
    if (input == nullptr) {
        std::cerr << "Failed to open input file: " << inputPath << "\n";
        return 1;
    }

    yyin = input;
    resetParseResult();
    if (yyparse() != 0) {
        std::fclose(input);
        std::cerr << "Parsing failed.\n";
        return 1;
    }
    std::fclose(input);

    ProgramNode* root = getParseResultRoot();
    if (root == nullptr) {
        std::cerr << "No AST root was produced by parser.\n";
        return 1;
    }

    SymbolTable symbolTable;
    ErrorHandler errorHandler;
    SemanticAnnotator annotator(symbolTable, errorHandler);
    annotator.annotate(root);

    if (errorHandler.hasErrors()) {
        for (const auto& err : errorHandler.errors()) {
            std::cerr << "Error at " << err.line << ":" << err.column << " - " << err.message << "\n";
        }
        return 1;
    }

    CodeGenerator generator;
    std::string cCode = generator.generate(root);

    std::ofstream out(outputPath);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return 1;
    }
    out << cCode;

    std::cout << "Generated C source: " << outputPath << "\n";
    return 0;
}
