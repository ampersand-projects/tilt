set(EXEC_BIND_SRC_FILES
    src/pyreg.cpp
    src/pyeng.cpp
    src/bindings.cpp
)

pybind11_add_module(exec ${EXEC_BIND_SRC_FILES})
target_include_directories(exec PUBLIC tilt include)
target_link_libraries(exec PRIVATE tilt)
