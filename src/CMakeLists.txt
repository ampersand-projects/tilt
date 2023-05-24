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
target_include_directories(tilt PUBLIC ${LLVM_INCLUDE_DIRS} ../include)
target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)

# Create bitcode for virtual instructions
set(VINSTR_SRC_FILE ${CMAKE_SOURCE_DIR}/src/pass/codegen/vinstr.cpp)
set(VINSTR_OUT_FILE ${CMAKE_BINARY_DIR}/vinstr.bc)
set(VINSTR_TMP_FILE /tmp/vinstr.bc)

add_custom_command(
    OUTPUT ${VINSTR_OUT_FILE}
    COMMAND clang++ -S -emit-llvm ${VINSTR_SRC_FILE} -o ${VINSTR_OUT_FILE}
            -I ${CMAKE_SOURCE_DIR}/include
    DEPENDS ${VINSTR_SRC_FILE}
)
add_custom_target(vinstr-bc DEPENDS ${VINSTR_OUT_FILE})
add_dependencies(tilt vinstr-bc)

# Copy bitcode to /tmp
add_custom_command(
  TARGET vinstr-bc
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          ${VINSTR_OUT_FILE}
          ${VINSTR_TMP_FILE}
)
add_custom_target(copy-bc DEPENDS vinstr-bc)
