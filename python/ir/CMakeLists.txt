pybind11_add_module(ir bindings.cpp)
target_include_directories(ir PUBLIC tilt)
target_link_libraries(ir PRIVATE tilt)
