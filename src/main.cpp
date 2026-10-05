#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Compiler.h"

namespace fs = std::filesystem;

namespace {

enum class Emit { Executable, Asm, Cpp };

struct Options {
    fs::path input;
    fs::path output;
    Emit emit = Emit::Executable;
    bool optimize = true;
    MasmOptions masm;
};

void printUsage() {
    std::cerr << "usage: bfc <input.bf> [-o output.exe] [--emit-asm] [--emit-cpp]\n"
                 "           [--no-opt] [--no-bounds-check]\n"
                 "  default: write <name>.asm, then run ml64 + link (needs a VS x64 Developer prompt)\n"
                 "  --emit-asm        only write the .asm file\n"
                 "  --emit-cpp        only write a .cpp file (C++ backend, no bounds check)\n"
                 "  --no-opt          skip the optimizer\n"
                 "  --no-bounds-check omit tape bounds checks in the generated assembly\n";
}

bool parseArgs(int argc, char** argv, Options& opts) {
    if (argc < 2) return false;
    opts.input = argv[1];
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) opts.output = argv[++i];
        else if (arg == "--emit-asm") opts.emit = Emit::Asm;
        else if (arg == "--emit-cpp") opts.emit = Emit::Cpp;
        else if (arg == "--no-opt") opts.optimize = false;
        else if (arg == "--no-bounds-check") opts.masm.boundsCheck = false;
        else return false;
    }
    if (opts.output.empty()) opts.output = fs::path(opts.input).replace_extension(".exe");
    return true;
}

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path.string());
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

void writeFile(const fs::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
    if (!out) throw std::runtime_error("cannot write " + path.string());
}

void assembleAndLink(const fs::path& asmPath, const fs::path& exePath) {
    // cmd.exe needs the whole command wrapped in an extra pair of quotes.
    const std::string cmd = "\"ml64 /nologo \"" + asmPath.string() + "\" /Fe\"" + exePath.string() +
                            "\" /link /nologo /SUBSYSTEM:CONSOLE /ENTRY:main kernel32.lib\"";
    if (std::system(cmd.c_str()) != 0) {
        throw std::runtime_error("ml64/link failed (run from a Visual Studio x64 Developer prompt)");
    }
}

void compile(const Options& opts) {
    Program program = parse(lex(readFile(opts.input)));
    if (opts.optimize) program = optimize(program);

    if (opts.emit == Emit::Cpp) {
        const fs::path cpp = fs::path(opts.output).replace_extension(".cpp");
        writeFile(cpp, emitCpp(program));
        std::cout << "wrote " << cpp.string() << "\n";
        return;
    }

    const fs::path asmPath = fs::path(opts.output).replace_extension(".asm");
    writeFile(asmPath, emitMasm(program, opts.masm));
    if (opts.emit == Emit::Asm) {
        std::cout << "wrote " << asmPath.string() << "\n";
        return;
    }

    assembleAndLink(asmPath, opts.output);
    std::cout << "built " << opts.output.string() << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    Options opts;
    if (!parseArgs(argc, argv, opts)) {
        printUsage();
        return 1;
    }
    try {
        compile(opts);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
