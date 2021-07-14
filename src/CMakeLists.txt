set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    ir/loop.cpp
    codegen/printer.cpp
    codegen/loopgen.cpp
    codegen/llvmgen.cpp
    engine/engine.cpp
    Formula.cpp
    Formula.h
)

add_library(tilt STATIC ${SRC_FILES})
add_dependencies(tilt easy-jit-core)

#add_executable(tilt_project ../main.cpp Formula.cpp Formula.h)  #######
#add_executable(tilt_run ../main.cpp Formula.cpp Formula.h) ######
#add_executable(tilt_run ${SRC_FILES}) ######

target_compile_options(tilt PUBLIC "SHELL:-Xclang -load" "SHELL:-Xclang ${CMAKE_BINARY_DIR}/third_party/easy_jit/bin/EasyJitPass.so" )
target_link_libraries(tilt EasyJitRuntime ${llvm_libs})

target_include_directories(tilt PUBLIC "${PROJECT_SOURCE_DIR}/include" ${PROJECT_SOURCE_DIR}/third_party/easy_jit/include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)