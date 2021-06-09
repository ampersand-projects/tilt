set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    ir/loop.cpp
    codegen/printer.cpp
    codegen/loopgen.cpp
    codegen/llvmgen.cpp
)

add_library(tilt STATIC ${SRC_FILES})
target_link_libraries(tilt ${llvm_libs})

target_include_directories(tilt PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror)
