#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Compiler.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL: " #c " line " << __LINE__ << "\n"; ++failures; } } while (0)

int main() {
    Program p = parse(lex("+++>>-"));
    CHECK(p.size() == 3);
    CHECK(p[0].op == Op::Add && p[0].arg == 3);
    CHECK(p[1].op == Op::Move && p[1].arg == 2);

    Program c = optimize(parse(lex("+++[-]")));
    CHECK(c.size() == 1 && c[0].op == Op::Set && c[0].arg == 0);

    Program m = optimize(parse(lex("[->+++<]")));
    CHECK(m.size() == 2);
    CHECK(m[0].op == Op::MulAdd && m[0].arg == 1 && m[0].factor == 3);
    CHECK(m[1].op == Op::Set && m[1].arg == 0);

    Program notFolded = optimize(parse(lex("[>+<]")));  // origin not decremented
    CHECK(notFolded.size() == 5);

    Program empty = optimize(parse(lex("[]")));  // must stay an infinite loop
    CHECK(empty.size() == 2);

    Program merged = optimize(parse(lex("+++[-]+++")));
    CHECK(merged.size() == 1 && merged[0].op == Op::Set && merged[0].arg == 3);

    CHECK(emitMasm(p).find("bf_oob") != std::string::npos);
    CHECK(emitMasm(p, {false}).find("bf_oob") == std::string::npos);

    bool threw = false;
    try { parse(lex("[[]")); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw);

    threw = false;
    try { parse(lex("]")); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw);

    CHECK(emitMasm(p).find("main PROC") != std::string::npos);
    CHECK(emitCpp(p).find("int main") != std::string::npos);

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
