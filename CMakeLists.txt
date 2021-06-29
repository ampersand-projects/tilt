cmake_minimum_required(VERSION 3.13.4)
project(tilt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++-11)

find_package(LLVM 11.1.0 REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_subdirectory(easy-just-in-time)

include_directories(${LLVM_INCLUDE_DIRS} include)
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core executionengine support native orcjit object)

add_subdirectory(src)
add_executable(main main.cpp)

target_compile_options(main PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${PROJECT_SOURCE_DIR}/build/easy-just-in-time/bin/EasyJitPass.so" )
target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/easy-just-in-time/include)

add_dependencies(main EasyJitPass)
target_link_libraries(main tilt ${llvm_libs} EasyJitRuntime)