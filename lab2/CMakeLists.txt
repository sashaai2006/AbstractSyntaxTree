cmake_minimum_required(VERSION 3.16)
project(lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_library(lab2_core
    parser/tokenizer.cpp
    parser/parser.cpp
    ast/ast.cpp
    analysis/subexpression_finder.cpp
    analysis/msp_checker.cpp
    util/subtree_utils.cpp
)

target_include_directories(lab2_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(lab2_tests
    tests/run_tests.cpp
)

target_link_libraries(lab2_tests PRIVATE lab2_core)

add_executable(lab2_app
    app/main.cpp
    app/astwidget.cpp
)

target_link_libraries(lab2_app PRIVATE lab2_core Qt6::Widgets)

