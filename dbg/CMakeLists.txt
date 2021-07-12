find_package(fmt CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(dbg
  "debugger.cpp"
  "lame_debugger.cpp")
target_link_libraries(dbg PRIVATE fmt::fmt-header-only)

#add_executable(dos_tests 
# "dos_test.cpp"
# )
#target_link_libraries(dos_tests cpu dos GTest::gtest_main)
#GTEST_DISCOVER_TESTS(dos_tests)
