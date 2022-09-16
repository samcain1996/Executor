cmake_minimum_required(VERSION 3.10)

project(ExecutorDemo)
project(Executor VERSION 1.0.0 DESCRIPTION "Run system commands and get the output within a program")

set(CMAKE_CXX_STANDARD 20)

add_executable(ExecutorDemo executor.cpp demo.cpp)
add_library(Executor SHARED executor.cpp)

set_target_properties(Executor PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER executor.h)
target_include_directories(Executor PRIVATE .)
install(TARGETS Executor
    LIBRARY DESTINATION .
    PUBLIC_HEADER DESTINATION .)

