set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    ir/loop.cpp
    codegen/printer.cpp
    codegen/loopgen.cpp
)

add_library(tilt STATIC ${SRC_FILES})

target_compile_options(tilt PRIVATE -Wall -Wextra -pedantic -Werror)
