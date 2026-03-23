cmake_minimum_required(VERSION 3.16)
project(pascc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(BISON)
find_package(FLEX)

set(CORE_SOURCES
    src/main.cpp
    src/ast.cpp
    src/semantic_annotator.cpp
    src/code_generator.cpp
    src/symbol_table.cpp
    src/error_handler.cpp
    src/codegen_utils.cpp
    src/debug_utils.cpp
)

if(BISON_FOUND AND FLEX_FOUND)
    BISON_TARGET(Parser
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
        ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp
    )

    FLEX_TARGET(Lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
        ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
    )

    ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

    add_executable(pascc
        ${CORE_SOURCES}
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
    )

    target_include_directories(pascc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
else()
    message(WARNING "Flex/Bison not found: building stub without lexer/parser generated sources.")
    add_executable(pascc ${CORE_SOURCES})
endif()

enable_testing()
add_test(NAME pascc_help COMMAND pascc --help)
