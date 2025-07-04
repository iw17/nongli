cmake_minimum_required(VERSION 3.10)

project(NongliTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /DNDEBUG")
    add_compile_options(/W4)
    add_compile_options(/wd4244)
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wpedantic)
    add_compile_options(-Wshadow)
endif()

add_executable(math_test "math.cpp")
add_executable(dati_test "dati.cpp")
add_executable(lunar_test "lunar.cpp")
add_executable(solar_test "solar.cpp")
add_executable(ganzhi_test "ganzhi.cpp")

add_test(NAME MathTest COMMAND math_test)
add_test(NAME DatiTest COMMAND dati_test)
add_test(NAME LunarTest COMMAND lunar_test)
add_test(NAME SolarTest COMMAND solar_test)
add_test(NAME GanzhiTest COMMAND ganzhi_test)