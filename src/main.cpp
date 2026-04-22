#include <cstdio>
#include <fstream>
#include <chrono>
#include <cstdlib>
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

    bool eventStreamEnabled()
    {
        const char *raw = std::getenv("PASCC_EVENT_STREAM");
        if (raw == nullptr)
        {
            return false;
        }
        std::string value(raw);
        return !value.empty() && value != "0" && value != "false" && value != "FALSE";
    }

    std::string escapeJson(const std::string &text)
    {
        std::string out;
        out.reserve(text.size() + 16);
        for (char ch : text)
        {
            switch (ch)
            {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out.push_back(ch);
                break;
            }
        }
        return out;
    }

    void emitEvent(const std::string &stage, const std::string &status, const std::string &message)
    {
        if (!eventStreamEnabled())
        {
            return;
        }
        std::cerr << "[PASCC_EVT]{\"stage\":\"" << escapeJson(stage)
                  << "\",\"status\":\"" << escapeJson(status)
                  << "\",\"message\":\"" << escapeJson(message)
                  << "\"}" << "\n";
    }

    void printUsage()
    {
        std::cout << "pascc - Pascal-S to C compiler (project skeleton)\n";
        std::cout << "Usage: pascc -i <input.pas> [-o output.c] [--lex] [--dump-tokens] [--parse] [--semantic] [--dump-annotated-ast] \n";
        std::cout << "运行会截止到参数所指步骤\n";
    }

    bool parseArgs(int argc, char *argv[], std::string &inputPath, std::string &outputPath,
                   bool &lexMode, bool &dumpTokens, bool &parseOnly, bool &semanticOnly,
                   bool &dumpAnnotatedAst,
                   bool &shouldExit, int &exitCode)
    {
        shouldExit = false;
        exitCode = 0;
        lexMode = false;
        dumpTokens = false;
        parseOnly = false;
        semanticOnly = false;
        dumpAnnotatedAst = false;
        outputPath.clear();

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
            if (arg == "--lex")
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
            if (arg == "--parse")
            {
                parseOnly = true;
                continue;
            }
            if (arg == "--semantic")
            {
                semanticOnly = true;
                continue;
            }
            if (arg == "--dump-annotated-ast")
            {
                dumpAnnotatedAst = true;
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
    bool semanticOnly = false;
    bool dumpAnnotatedAst = false;
    bool shouldExit = false;
    int exitCode = 0;

    if (!parseArgs(argc, argv, inputPath, outputPath, lexMode, dumpTokens, parseOnly,
                   semanticOnly, dumpAnnotatedAst, shouldExit, exitCode))
    {
        emitEvent("init", "failed", "Argument parsing failed");
        pasccLog("init", PasccLogLevel::Error, "参数解析失败");
        return 1;
    }
    if (shouldExit)
    {
        emitEvent("init", "done", "Exit requested by arguments");
        pasccLog("init", PasccLogLevel::Info, "参数要求提前退出");
        return exitCode;
    }

    emitEvent("init", "running", "Arguments parsed");
    pasccLog("init", PasccLogLevel::Info, std::string("输入文件: ") + inputPath);

    // 若未指定 -o，自动生成与输入文件同名、同目录，仅扩展名改为.c
    if (outputPath.empty() && !inputPath.empty()) {
        size_t lastDot = inputPath.find_last_of('.');
        std::string base = (lastDot == std::string::npos) ? inputPath : inputPath.substr(0, lastDot);
        outputPath = base + ".c";
    }
    pasccLog("output", PasccLogLevel::Debug, std::string("输出文件: ") + outputPath);

    FILE *input = std::fopen(inputPath.c_str(), "r");
    if (input == nullptr)
    {
        emitEvent("init", "failed", std::string("Failed to open input file: ") + inputPath);
        pasccLog("init", PasccLogLevel::Error, std::string("无法打开输入文件: ") + inputPath);
        std::cerr << "Failed to open input file: " << inputPath << "\n";
        return 1;
    }

    emitEvent("init", "done", std::string("Opened input: ") + inputPath);
    pasccLog("init", PasccLogLevel::Info, "输入文件打开成功");

    yyin = input;
    lexerResetState();
    lexerClearErrors();

    if (lexMode)
    {
        emitEvent("lexer", "running", dumpTokens ? "Lexical mode started (token dump)" : "Lexical mode started");
        pasccLog("lexer", PasccLogLevel::Info, dumpTokens ? "进入词法模式（含规则命中输出）" : "进入词法模式");
        runLexMode(dumpTokens);
        std::fclose(input);

        if (lexerErrorCount() > 0)
        {
            emitEvent("lexer", "failed", "Lexical analysis completed with errors");
            pasccLog("lexer", PasccLogLevel::Error, "词法分析结束，存在错误");
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
        pasccLog("lexer", PasccLogLevel::Info, std::string("词法分析完成，token数=") + std::to_string(lexerTokenCount()));
        emitEvent("lexer", "done", "Lexical analysis succeeded");
        return 0;
    }

    emitEvent("lexer", "running", "Lexical scanning for parser input");
    pasccLog("lexer", PasccLogLevel::Info, "开始为语法阶段提供token流");
    resetParseResult();
    emitEvent("parser", "running", "Syntax analysis started");
    pasccLog("parser", PasccLogLevel::Info, "语法分析开始");
    if (yyparse() != 0)
    {
        std::fclose(input);
        emitEvent("parser", "failed", "Parser returned non-zero");
        pasccLog("parser", PasccLogLevel::Error, "yyparse返回非零");
        std::cerr << "Parsing failed.\n";
        return 1;
    }
    if (getParseErrorCount() > 0)
    {
        std::fclose(input);
        emitEvent("parser", "failed", "Parser reported syntax errors");
        pasccLog("parser", PasccLogLevel::Error, std::string("语法错误数量=") + std::to_string(getParseErrorCount()));
        std::cerr << "Parsing failed.\n";
        return 1;
    }
    std::fclose(input);
    emitEvent("lexer", "done", "Lexical scanning succeeded");
    emitEvent("parser", "done", "Syntax analysis succeeded");
    pasccLog("parser", PasccLogLevel::Info, "语法分析成功");

    ProgramNode *root = getParseResultRoot();
    if (root == nullptr)
    {
        emitEvent("parser", "failed", "No AST root produced");
        pasccLog("parser", PasccLogLevel::Error, "未生成AST根节点");
        std::cerr << "No AST root was produced by parser.\n";
        return 1;
    }
    pasccLog("parser", PasccLogLevel::Debug, std::string("AST根节点子节点数=") + std::to_string(root->children.size()));

    if (parseOnly)
    {
        emitEvent("compiler", "done", "Stopped after parse stage");
        pasccLog("compiler", PasccLogLevel::Info, "按参数在语法阶段停止");
        std::cout << "Parse succeeded.\n";
        
        printAstNode(root);
        
        return 0;
    }

    emitEvent("semantic", "running", "Semantic analysis started");
    pasccLog("semantic", PasccLogLevel::Info, "语义分析开始");
    SymbolTable symbolTable;
    semantic_register::preregisterBuiltins(symbolTable);
    pasccLog("semantic", PasccLogLevel::Debug, "内建符号预注册完成");
    ErrorHandler errorHandler;
    SemanticAnnotator annotator(symbolTable, errorHandler);
    annotator.annotate(root);

    if (dumpAnnotatedAst)
    {
        std::cout << "Annotated AST:\n";
        printAnnotatedAstNode(root);
    }

    if (errorHandler.hasErrors())
    {
        emitEvent("semantic", "failed", "Semantic analysis reported errors");
        pasccLog("semantic", PasccLogLevel::Error, std::string("语义分析失败，错误数=") + std::to_string(errorHandler.errors().size()));
        for (const auto &err : errorHandler.errors())
        {
            std::cerr << "Error at " << err.line << ":" << err.column << " - " << err.message << "\n";
        }
        return 1;
    }

    emitEvent("semantic", "done", "Semantic analysis succeeded");
    pasccLog("semantic", PasccLogLevel::Info, "语义分析成功");

    if (semanticOnly)
    {
        emitEvent("compiler", "done", "Stopped after semantic stage");
        pasccLog("compiler", PasccLogLevel::Info, "按参数在语义阶段停止");
        std::cout << "Semantic analysis succeeded.\n";
        return 0;
    }

    emitEvent("codegen", "running", "Code generation started");
    pasccLog("codegen", PasccLogLevel::Info, "代码生成开始");
    CodeGenerator generator;
    std::string cCode = generator.generate(root);
    emitEvent("codegen", "done", "Code generation succeeded");
    pasccLog("codegen", PasccLogLevel::Info, std::string("代码生成完成，输出长度=") + std::to_string(cCode.size()));

    emitEvent("output", "running", std::string("Writing output file: ") + outputPath);
    pasccLog("output", PasccLogLevel::Info, "开始写入输出文件");
    std::ofstream out(outputPath);
    if (!out)
    {
        emitEvent("output", "failed", std::string("Failed to open output file: ") + outputPath);
        pasccLog("output", PasccLogLevel::Error, std::string("无法打开输出文件: ") + outputPath);
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return 1;
    }
    out << cCode;
    emitEvent("output", "done", "Output file written");
    pasccLog("output", PasccLogLevel::Info, "输出文件写入完成");

    std::cout << "Generated C source: " << outputPath << "\n";
    emitEvent("compiler", "done", "Compilation finished successfully");
    pasccLog("compiler", PasccLogLevel::Info, "编译流程完成");
    return 0;
}
