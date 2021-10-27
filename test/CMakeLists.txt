set(TEST_FILES
    src/basic_tests.cpp
    src/test_base.cpp
    src/test_query.cpp
)

add_executable(tilt_test ${TEST_FILES})
target_include_directories(tilt_test PUBLIC include)
target_link_libraries(tilt_test gtest_main tilt)

include(GoogleTest)
gtest_discover_tests(tilt_test)
