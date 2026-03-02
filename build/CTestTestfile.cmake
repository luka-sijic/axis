# CMake generated Testfile for 
# Source directory: /Users/based/Documents/repos/axis
# Build directory: /Users/based/Documents/repos/axis/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[smoke]=] "/Users/based/Documents/repos/axis/build/axis_smoke")
set_tests_properties([=[smoke]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/based/Documents/repos/axis/CMakeLists.txt;56;add_test;/Users/based/Documents/repos/axis/CMakeLists.txt;0;")
subdirs("tests")
