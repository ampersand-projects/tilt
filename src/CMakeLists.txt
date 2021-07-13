set(SRC_FILES
    ir/node.cpp
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    ir/loop.cpp
    codegen/printer.cpp
    codegen/loopgen.cpp
    codegen/llvmgen.cpp
    engine/engine.cpp
    builder/tilder.cpp
)

add_library(tilt STATIC ${SRC_FILES})
add_dependencies(tilt easy-jit-core)

target_compile_options(tilt PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${CMAKE_BINARY_DIR}/third_party/easy_jit/bin/EasyJitPass.so")
target_link_libraries(tilt EasyJitRuntime ${llvm_libs})

target_include_directories(tilt PUBLIC "${PROJECT_SOURCE_DIR}/include" ${PROJECT_SOURCE_DIR}/third_party/easy_jit/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
