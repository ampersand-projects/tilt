cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(tilt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party/googletest)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)

option(BUILD_PYTILT "Option description" OFF)
if(BUILD_PYTILT)
    add_subdirectory(third_party/pybind11)
    add_subdirectory(python)
endif()