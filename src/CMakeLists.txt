set(SRC_FILES
    ir/ir.cpp
    builder/tilder.cpp
    pass/printer.cpp
    pass/codegen/loopgen.cpp
    pass/codegen/llvmgen.cpp
    pass/codegen/vinstr.cpp
    engine/engine.cpp
)

add_library(tilt STATIC ${SRC_FILES})

target_link_libraries(tilt ${llvm_libs})
target_include_directories(tilt PUBLIC ${LLVM_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)


# Code generation for JIT

# Compile vinstr to LLVM IR
execute_process(COMMAND clang++ -S -emit-llvm ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr.cpp -I ${CMAKE_SOURCE_DIR}/include/ -o ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll)

# Load LLVM IR as string
file(READ ${CMAKE_CURRENT_BINARY_DIR}/vinstr.ll VINSTR_IR NEWLINE_CONSUME)

# Insert into file
configure_file(${CMAKE_SOURCE_DIR}/include/tilt/pass/codegen/vinstr_str.h.in ${CMAKE_SOURCE_DIR}/include/tilt/pass/codegen/vinstr_str.h @ONLY)
