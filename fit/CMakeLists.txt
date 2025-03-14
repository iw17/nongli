cmake_minimum_required(VERSION 3.10)

project(nongli-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/O2)
    add_compile_options(/W4)
    add_compile_options(/wd4244)
else()
    add_compile_options(-O2)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wpedantic)
endif()

add_executable(test test.cpp)
