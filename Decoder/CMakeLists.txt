cmake_minimum_required(VERSION 3.16)
project(opto_decoder)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(prog main.cpp decoder.cpp morse_code.h morse_code.cpp stream.hpp stream.cpp platform_host.hpp)

target_compile_options(prog PRIVATE -g -Og -Wall -pedantic)
target_compile_definitions(prog PRIVATE HOST_BUILD)
