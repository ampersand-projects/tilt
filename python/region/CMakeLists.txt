set(REG_BIND_SRC_FILES
    src/pyreg.cpp
    src/bindings.cpp
)

pybind11_add_module(region ${REG_BIND_SRC_FILES})
target_include_directories(region PUBLIC tilt include)
target_link_libraries(region PRIVATE tilt)
