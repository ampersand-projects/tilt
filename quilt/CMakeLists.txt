set(SRC_FILES
    quilt.cpp
)

add_library(quilt STATIC ${SRC_FILES})

target_include_directories(quilt PUBLIC "${PROJECT_SOURCE_DIR}/include")
#target_link_libraries(quilt tilt)
