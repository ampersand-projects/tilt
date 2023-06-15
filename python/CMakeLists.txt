cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(pytilt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM 11.0.0 REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(pybind11)

add_library(tilt STATIC IMPORTED)
set_target_properties(tilt PROPERTIES
    IMPORTED_LOCATION ${TILT_DIRECTORY}/build/src/libtilt.so)

pybind11_add_module(pytilt main.cpp)
target_include_directories(pytilt PUBLIC ${LLVM_INCLUDE_DIRS} ${TILT_DIRECTORY}/include)
target_link_libraries(pytilt PRIVATE tilt ${llvm_libs})