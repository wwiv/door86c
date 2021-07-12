#############################################################################
#
# Door86 Emulator
#
find_package(fmt CONFIG REQUIRED)

add_executable(door86 door86.cpp)
target_link_libraries(door86 PRIVATE fmt::fmt-header-only bios core dbg dos cpu)
target_include_directories(door86 PRIVATE ${CMAKE_SOURCE_DIR}/deps/wwiv})

