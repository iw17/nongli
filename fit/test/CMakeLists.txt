cmake_minimum_required(VERSION 3.10)

project(NongliTest)
enable_testing()

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
    add_compile_options(-Wshadow)
endif()

add_executable(nltest "test.cpp")
add_test(NAME NongliTest COMMAND nltest)