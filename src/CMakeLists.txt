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

# Config for JIT
set(NO_EASYJIT true)
if(${NO_EASYJIT})
    add_compile_definitions(NO_EASYJIT)
endif()

# Load required files
file(STRINGS ../include/tilt/base/ctype.h VINSTR_TYPE NEWLINE_CONSUME)
file(STRINGS ../include/tilt/pass/codegen/vinstr.h VINSTR_HEADER NEWLINE_CONSUME)
file(STRINGS ../src/pass/codegen/vinstr.cpp VINSTR_SRC NEWLINE_CONSUME)

# Replace preprocessors macros
string(REGEX REPLACE "(^|\n)(#[^\n]*)(\n|$)" "" FILTERED_VINSTR_TYPE ${VINSTR_TYPE})
string(REGEX REPLACE "(^|\n)(#[^\n]*)(\n|$)" "" FILTERED_VINSTR_HEADER ${VINSTR_HEADER})
string(REGEX REPLACE "(^|\n)(#[^\n]*)(\n|$)" "" FILTERED_VINSTR_SRC ${VINSTR_SRC})

# Create code contents
set(VINSTR "${FILTERED_VINSTR_TYPE}\n${FILTERED_VINSTR_HEADER}\n${FILTERED_VINSTR_SRC}")

# Insert into file
configure_file(../include/tilt/pass/codegen/vinstr_str.h.in ${CMAKE_SOURCE_DIR}/include/tilt/pass/codegen/vinstr_str.h @ONLY)
