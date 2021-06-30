find_package(fmt CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(dos 
  "exe.cpp"
  "psp.cpp"
   "dos.cpp")
target_link_libraries(dos PRIVATE fmt::fmt-header-only)

add_executable(dos_tests 
 "psp_test.cpp"
 )
target_link_libraries(dos_tests cpu dos GTest::gtest_main)
GTEST_DISCOVER_TESTS(dos_tests)
