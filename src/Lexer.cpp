#include "Lexer.h"

std::vector<Token> lex(const std::string& source) {
    std::vector<Token> tokens;
    int line = 1, column = 0;
    for (char c : source) {
        ++column;
        if (c == '\n') {
            ++line;
            column = 0;
            continue;
        }
        TokenKind kind;
        switch (c) {
            case '+': kind = TokenKind::Inc; break;
            case '-': kind = TokenKind::Dec; break;
            case '>': kind = TokenKind::Right; break;
            case '<': kind = TokenKind::Left; break;
            case '.': kind = TokenKind::Out; break;
            case ',': kind = TokenKind::In; break;
            case '[': kind = TokenKind::LoopBegin; break;
            case ']': kind = TokenKind::LoopEnd; break;
            default: continue;
        }
        tokens.push_back({kind, line, column});
    }
    return tokens;
}
