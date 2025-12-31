cmake_minimum_required(VERSION 3.5.1)

project(dbg)

set(EXEC dbg)

set(CMAKE_CXX_STANDARD 20)

include_directories(includes)

file(GLOB SOURCES src/*.cc exts/linenoise/*.c)

add_executable(${EXEC} ${SOURCES})


