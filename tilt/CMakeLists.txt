set(SRC_FILES
    src/ir/ir.cpp
    src/builder/tilder.cpp
    src/pass/printer.cpp
    src/pass/codegen/loopgen.cpp
    src/pass/codegen/llvmgen.cpp
    src/pass/codegen/vinstr.cpp
    src/engine/engine.cpp
)

add_library(tilt STATIC ${SRC_FILES})
add_dependencies(tilt easy-jit-core)

target_compile_options(tilt PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${easyjit_lib}")
target_link_libraries(tilt EasyJitRuntime ${llvm_libs})

target_include_directories(tilt PUBLIC ${LLVM_INCLUDE_DIRS} include ../third_party/easy_jit/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
