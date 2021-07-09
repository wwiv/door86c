#############################################################################
#
# Door86 Emulator
#
find_package(fmt CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(cpu 
  "memory.cpp"
  "x86/decoder.cpp"
  "x86/cpu.cpp"
  "x86/rmm.cpp"
   "io.cpp" "x86/cpu_core.cpp")
target_link_libraries(cpu PRIVATE core fmt::fmt-header-only)
#target_compile_options(cpu PUBLIC /FAcs)

add_library(cpu_fixtures
  "x86/cpu_fixture.cpp"
  )
target_link_libraries(cpu_fixtures PRIVATE core fmt::fmt-header-only)

add_executable(cpu_tests 
 "x86/cpu_test.cpp"
)
target_link_libraries(cpu_tests cpu cpu_fixtures GTest::gtest_main)
GTEST_DISCOVER_TESTS(cpu_tests)

add_executable(decoder_tests 
 "x86/decoder_test.cpp"
)
target_link_libraries(decoder_tests cpu cpu_fixtures GTest::gtest_main)
GTEST_DISCOVER_TESTS(decoder_tests)

add_executable(memory_tests 
 "memory_test.cpp"
)
target_link_libraries(memory_tests cpu GTest::gtest_main)
GTEST_DISCOVER_TESTS(memory_tests)

add_executable(rmm_tests 
 "x86/rmm_test.cpp"
)
target_link_libraries(rmm_tests cpu cpu_fixtures GTest::gtest_main)
GTEST_DISCOVER_TESTS(rmm_tests)

