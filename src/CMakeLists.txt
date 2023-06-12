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
add_dependencies(tilt gen-vinstr)

# Generate vinstr IR for JIT
add_custom_target(gen-vinstr DEPENDS ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr.cpp)
add_custom_command(
    TARGET gen-vinstr
    COMMAND bash ${CMAKE_SOURCE_DIR}/gen-vinstr.sh ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)
