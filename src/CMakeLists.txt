set(SRC_FILES
    ir/op.cpp
    ir/expr.cpp
    ir/lstream.cpp
    codegen/printer.cpp
)

add_library(tillib STATIC ${SRC_FILES})

target_compile_options(tillib PRIVATE -Wall -Wextra -pedantic -Werror)
