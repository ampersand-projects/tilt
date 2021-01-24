set(SRC_FILES
    core/op.cpp
    )

add_library(tillib STATIC ${SRC_FILES})

target_compile_options(tillib PRIVATE -Wall -Wextra -pedantic -Werror)
