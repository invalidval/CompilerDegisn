#include <cstdio>
#include <fstream>
#include <chrono>
#include <iostream>
#include <string>

#include "parser.hpp"

#include "code_generator.h"
#include "error_handler.h"
#include "parser_bridge.h"
#include "semantic_annotator.h"
#include "semantic_register.h"
#include "symbol_table.h"
#include "debug_utils.h"

int yylex(void);

void lexerResetState();
const char *lexerLastLexeme();
const char *lexerLastType();
const char *lexerLastRule();
int lexerLastLine();
int lexerLastColumn();
void lexerClearErrors();
int lexerErrorCount();
const CompileError *lexerErrorAt(int index);
int lexerAllocatedStringCount();
int lexerTokenCount();

namespace
{

    void printUsage()
    {
        std::cout << "pascc - Pascal-S to C compiler (project skeleton)\n";
        std::cout << "Usage: pascc -i <input.pas> [-o output.c] [--lex-only] [--dump-tokens] [--parse-only]\n";
        std::cout << "注意，--parse-only 也会执行词法分析\n";
    }

    bool parseArgs(int argc, char *argv[], std::string &inputPath, std::string &outputPath,
                   bool &lexMode, bool &dumpTokens, bool &parseOnly, bool &shouldExit, int &exitCode)
    {
        shouldExit = false;
        exitCode = 0;
        lexMode = false;
        dumpTokens = false;
        parseOnly = false;
        outputPath = "out.c";

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--help")
            {
                printUsage();
                shouldExit = true;
                exitCode = 0;
                return true;
            }
            if (arg == "-i" && i + 1 < argc)
            {
                inputPath = argv[++i];
                continue;
            }
            if (arg == "-o" && i + 1 < argc)
            {
                outputPath = argv[++i];
                continue;
            }
            if (arg == "--lex-only")
            {
                lexMode = true;
                continue;
            }
            if (arg == "--dump-tokens")
            {
                lexMode = true;
                dumpTokens = true;
                continue;
            }
            if (arg == "--parse-only")
            {
                parseOnly = true;
                continue;
            }
        }

        if (inputPath.empty())
        {
            printUsage();
            shouldExit = true;
            exitCode = 1;
            return true;
        }
        return true;
    }

    std::string escapeLexeme(const std::string &text)
    {
        std::string out;
        out.reserve(text.size());
        for (char ch : text)
        {
            switch (ch)
            {
            case '\n':
                out += "\\\\n";
                break;
            case '\t':
                out += "\\\\t";
                break;
            case '\r':
                out += "\\\\r";
                break;
            default:
                out.push_back(ch);
                break;
            }
        }
        return out;
    }

    void runLexMode(bool dumpTokens)
    {
        auto startTime = std::chrono::steady_clock::now();
        if (dumpTokens)
        {
            std::cout << "Type, Lexeme, Line, Column, Rule\n";
        }
        else
        {
            std::cout << "Type, Lexeme, Line, Column\n";
        }
        for (int token = yylex(); token != 0; token = yylex())
        {
            std::string type = lexerLastType();
            if (type.empty())
            {
                if (token == PROGRAM)
                {
                    type = "Keyword";
                }
                else if (token == IDENTIFIER)
                {
                    type = "Identifier";
                }
                else if (token == NUMBER)
                {
                    type = "Number";
                }
                else if (token == CHARACTER)
                {
                    type = "Character";
                }
                else if (token == STRING)
                {
                    type = "String";
                }
                else if (token < 256)
                {
                    type = "Symbol";
                }
                else
                {
                    type = "Unknown";
                }
            }
            std::cout << type << ", "
                      << escapeLexeme(lexerLastLexeme()) << ", "
                      << lexerLastLine() << ", "
                      << lexerLastColumn();
            if (dumpTokens)
            {
                std::cout << ", " << lexerLastRule();
            }
            std::cout << "\n";
        }

        if (dumpTokens)
        {
            auto endTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            std::cerr << "[dump] tokens=" << lexerTokenCount()
                      << ", strdup_calls=" << lexerAllocatedStringCount()
                      << ", elapsed_us=" << elapsed.count() << "\n";
        }
    }

} // namespace

int main(int argc, char *argv[])
{
    std::string inputPath;
    std::string outputPath;
    bool lexMode = false;
    bool dumpTokens = false;
    bool parseOnly = false;
    bool shouldExit = false;
    int exitCode = 0;
    if (!parseArgs(argc, argv, inputPath, outputPath, lexMode, dumpTokens, parseOnly, shouldExit, exitCode))
    {
        return 1;
    }
    if (shouldExit)
    {
        return exitCode;
    }

    FILE *input = std::fopen(inputPath.c_str(), "r");
    if (input == nullptr)
    {
        std::cerr << "Failed to open input file: " << inputPath << "\n";
        return 1;
    }

    yyin = input;
    lexerResetState();
    lexerClearErrors();

    if (lexMode)
    {
        runLexMode(dumpTokens);
        std::fclose(input);

        if (lexerErrorCount() > 0)
        {
            for (int i = 0; i < lexerErrorCount(); ++i)
            {
                const CompileError *err = lexerErrorAt(i);
                if (err != nullptr)
                {
                    std::cerr << "Error at " << err->line << ":" << err->column
                              << " - " << err->message << "\n";
                }
            }
            return 1;
        }
        return 0;
    }

    resetParseResult();
    if (yyparse() != 0)
    {
        std::fclose(input);
        std::cerr << "Parsing failed.\n";
        return 1;
    }
    if (getParseErrorCount() > 0)
    {
        std::fclose(input);
        std::cerr << "Parsing failed.\n";
        return 1;
    }
    std::fclose(input);

    ProgramNode *root = getParseResultRoot();
    if (root == nullptr)
    {
        std::cerr << "No AST root was produced by parser.\n";
        return 1;
    }

    if (parseOnly)
    {
        std::cout << "Parse succeeded.\n";
        
        printAstNode(root);
        
        return 0;
    }

    SymbolTable symbolTable;
    semantic_register::preregisterBuiltins(symbolTable);
    ErrorHandler errorHandler;
    SemanticAnnotator annotator(symbolTable, errorHandler);
    annotator.annotate(root);

    if (errorHandler.hasErrors())
    {
        for (const auto &err : errorHandler.errors())
        {
            std::cerr << "Error at " << err.line << ":" << err.column << " - " << err.message << "\n";
        }
        return 1;
    }

    CodeGenerator generator;
    std::string cCode = generator.generate(root);

    std::ofstream out(outputPath);
    if (!out)
    {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return 1;
    }
    out << cCode;

    std::cout << "Generated C source: " << outputPath << "\n";
    return 0;
}
