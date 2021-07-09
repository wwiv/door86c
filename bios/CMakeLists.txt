find_package(fmt CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_library(bios 
  "bios.cpp"
)
target_link_libraries(bios PRIVATE fmt::fmt-header-only)

#add_executable(bios_tests 
# "bios_test.cpp"
# )
#target_link_libraries(bios_tests cpu bios GTest::gtest_main)
#GTEST_DISCOVER_TESTS(bios_tests)
