set(SRC_FILES
    quilt.cpp
)

add_library(quilt STATIC ${SRC_FILES})

target_link_libraries(quilt tilt)
target_include_directories(quilt PUBLIC "${PROJECT_SOURCE_DIR}/include")