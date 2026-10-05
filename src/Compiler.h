#pragma once
#include <string>
#include <vector>

#include "Lexer.h"

enum class Op {
    Add,        // tape[p] += arg
    Move,       // p += arg
    Set,        // tape[p] = arg
    MulAdd,     // tape[p + arg] += tape[p] * factor
    Out,
    In,
    LoopBegin,  // while (tape[p]) {
    LoopEnd     // }
};

struct Instr {
    Op op;
    int arg = 0;
    int factor = 0;  // MulAdd only
};

using Program = std::vector<Instr>;

struct MasmOptions {
    bool boundsCheck = true;  // exit with code 1 when the data pointer leaves the tape
};

// Throws std::runtime_error on unmatched brackets.
Program parse(const std::vector<Token>& tokens);

// Folds clear/multiply loops and merges redundant operations.
Program optimize(const Program& program);

std::string emitMasm(const Program& program, const MasmOptions& options = {});
std::string emitCpp(const Program& program);
