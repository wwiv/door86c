#############################################################################
#
# Door86 Emulator
#
find_package(fmt CONFIG REQUIRED)

add_executable(dis dis.cpp)
target_link_libraries(dis PRIVATE fmt::fmt-header-only core)
target_include_directories(dis PRIVATE ${CMAKE_SOURCE_DIR}/deps/wwiv})

