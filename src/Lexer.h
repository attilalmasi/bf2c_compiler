#pragma once
#include <string>
#include <vector>

enum class TokenKind { Inc, Dec, Right, Left, Out, In, LoopBegin, LoopEnd };

struct Token {
    TokenKind kind;
    int line;
    int column;
};

// Non-command characters are comments and are skipped.
std::vector<Token> lex(const std::string& source);
