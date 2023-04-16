cmake_minimum_required(VERSION 3.14)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_INCLUDE ${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src)

set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output)