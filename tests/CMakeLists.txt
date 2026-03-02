# Fetch Google Test (or FindPackage if you prefer)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Create test executable from all .cpp files in this dir
file(GLOB TEST_SOURCES "*.cpp")
add_executable(axis_tests ${TEST_SOURCES})

target_link_libraries(axis_tests 
    PRIVATE 
        axis::axis 
        GTest::gtest_main
)

# Register with CTest
include(GoogleTest)
gtest_discover_tests(axis_tests)
