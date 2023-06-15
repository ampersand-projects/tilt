set(SRC_FILES
    ir/ir.cpp
    builder/tilder.cpp
    pass/printer.cpp
    pass/codegen/loopgen.cpp
    pass/codegen/llvmgen.cpp
    pass/codegen/vinstr.cpp
    engine/engine.cpp
)

find_package(LLVM 11.0.0 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs native orcjit mcjit objcarcopts)

# Generate vinstr IR for JIT
#
# We have two commands that run scripts/gen_vinstr.sh because
# execute_process is for configure (cmake) and target is for build (make)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vinstr_str.cpp
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/gen_vinstr.sh ${CMAKE_CXX_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS pass/codegen/vinstr.cpp
)

add_library(tilt STATIC ${SRC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/vinstr_str.cpp)

target_link_libraries(tilt ${llvm_libs})
target_include_directories(tilt PUBLIC ${LLVM_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
