set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    codegen/printer.cpp
)

add_library(tiltlib STATIC ${SRC_FILES})

target_compile_options(tiltlib PRIVATE -Wall -Wextra -pedantic -Werror)
