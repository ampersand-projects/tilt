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
add_dependencies(tilt easy-jit-core)

target_compile_options(tilt PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${easyjit_lib}")
target_link_libraries(tilt EasyJitRuntime ${llvm_libs})

target_include_directories(tilt PUBLIC ${LLVM_INCLUDE_DIRS} ../include ../third_party/easy_jit/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
