cmake_minimum_required(VERSION 3.13.4)
project(tilt)

#find_package(LLVM REQUIRED CONFIG)

#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

#include_directories(${LLVM_INCLUDE_DIRS} include)
include_directories(include)
#add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(src)

# Now build our tools
add_executable(main main.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
#llvm_map_components_to_libnames(llvm_libs core executionengine support native orcjit object)

# Link against LLVM libraries
#target_link_libraries(tilt ${llvm_libs} tiltib)
target_link_libraries(main tilt)

#target_include_directories(tilt PUBLIC "${PROJECT_SOURCE_DIR}/include")
