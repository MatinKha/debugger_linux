cmake_minimum_required(VERSION 3.5.1)

project(dbg)

set(EXEC dbg)

set(CMAKE_CXX_STANDARD 20)

set(LIBELFIN_DIR ${CMAKE_SOURCE_DIR}/exts/libelfin)

include_directories(includes)

file(GLOB SOURCES src/*.cc exts/linenoise/*.c exts/libelfin)




add_executable(${EXEC} ${SOURCES})



target_include_directories(dbg PRIVATE
    ${LIBELFIN_DIR}
)

target_link_libraries(dbg
    ${LIBELFIN_DIR}/elf/libelf++.a
    ${LIBELFIN_DIR}/dwarf/libdwarf++.a
    elf
)