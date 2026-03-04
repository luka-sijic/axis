cmake_minimum_required(VERSION 3.20)
project(axis VERSION 0.1.0 LANGUAGES CXX)

add_library(axis INTERFACE)
add_library(axis::axis ALIAS axis)

target_include_directories(axis
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(axis INTERFACE cxx_std_23)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS axis
  EXPORT axisTargets
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT axisTargets
  NAMESPACE axis::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/axis
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/blibConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/axisConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/axisConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/axis
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/axisConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/axisConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/axis
)

option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()

    add_executable(axis_smoke src/main.cpp)
    target_link_libraries(axis_smoke PRIVATE axis::axis)
    target_link_libraries(axis_smoke PRIVATE uring)
    add_test(NAME smoke COMMAND axis_smoke)

    add_subdirectory(tests)
endif()
