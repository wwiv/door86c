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
  "x86/core.cpp"
  )
target_link_libraries(cpu PRIVATE fmt::fmt)

add_executable(core_tests 
 "x86/core_test.cpp"
)
target_link_libraries(core_tests cpu GTest::gtest_main)
GTEST_DISCOVER_TESTS(core_tests)

add_executable(decoder_tests 
 "x86/decoder_test.cpp"
)
target_link_libraries(decoder_tests cpu GTest::gtest_main)
GTEST_DISCOVER_TESTS(decoder_tests)
