pybind11_add_module(ir src/bindings/ir.cpp)
target_include_directories(ir PUBLIC tilt)
target_link_libraries(ir PRIVATE tilt)

pybind11_add_module(utils src/bindings/utils.cpp)
target_include_directories(utils PUBLIC tilt)
target_link_libraries(utils PRIVATE tilt)

set(EXEC_BIND_SRC_FILES
    src/pyreg.cpp
    src/pyeng.cpp
    src/bindings/exec.cpp
)
pybind11_add_module(exec ${EXEC_BIND_SRC_FILES})
target_include_directories(exec PUBLIC tilt include)
target_link_libraries(exec PRIVATE tilt)
