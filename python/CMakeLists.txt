find_package(pybind11 CONFIG)

pybind11_add_module(pytilt main.cpp)
target_include_directories(pytilt PUBLIC tilt)
target_link_libraries(pytilt PRIVATE tilt)