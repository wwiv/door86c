find_package(fmt CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(dbg
  "debugger.cpp"
  "gdb_debugger.cpp"
  "lame_debugger.cpp")
target_link_libraries(dbg PRIVATE core fmt::fmt-header-only)

add_executable(gdb_debugger_tests 
 "gdb_debugger_test.cpp"
 )
target_link_libraries(gdb_debugger_tests cpu dbg  GTest::gtest_main)
GTEST_DISCOVER_TESTS(gdb_debugger_tests)
