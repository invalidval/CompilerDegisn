#ifndef PASCC_TOKEN_H
#define PASCC_TOKEN_H

#include <string>

enum class TokenType {
    Keyword,
    Identifier,
    Number,
    Character,
    Operator,
    Delimiter,
    EndOfFile,
    Invalid
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

#endif  // PASCC_TOKEN_H
