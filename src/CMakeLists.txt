set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    ir/loop.cpp
    codegen/printer.cpp
    codegen/loopgen.cpp
    codegen/llvmgen.cpp
    engine/engine.cpp
)

add_library(tilt STATIC ${SRC_FILES})
add_dependencies(tilt EasyJitPass)

target_compile_options(tilt PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${PROJECT_SOURCE_DIR}/build/easy_jit/bin/EasyJitPass.so" )
target_link_libraries(tilt ${llvm_libs} EasyJitRuntime)

target_include_directories(tilt PUBLIC "${PROJECT_SOURCE_DIR}/include" ${PROJECT_SOURCE_DIR}/easy_jit/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)