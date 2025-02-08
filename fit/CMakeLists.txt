cmake_minimum_required(VERSION "3.10")

project(
    "Nongli"
    VERSION "2.1.0"
    DESCRIPTION "C++ library for Chinese calendar"
    LANGUAGES "CXX"
)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

add_executable(test.out test.cpp)
