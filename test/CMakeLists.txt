set(TEST_FILES
    basic_tests.cpp
)

add_executable(tilt_test ${TEST_FILES})
target_link_libraries(tilt_test gtest_main tilt)

include(GoogleTest)
gtest_discover_tests(tilt_test)
