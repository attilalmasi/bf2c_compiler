cmake_minimum_required(VERSION 3.20)
project(BrainfuckCompiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(bfcore src/Lexer.cpp src/Compiler.cpp)
target_include_directories(bfcore PUBLIC src)

add_executable(bfc src/main.cpp)
target_link_libraries(bfc PRIVATE bfcore)

enable_testing()
add_executable(bf_tests tests/test_compiler.cpp)
target_link_libraries(bf_tests PRIVATE bfcore)
add_test(NAME bf_tests COMMAND bf_tests)
