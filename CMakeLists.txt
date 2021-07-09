#############################################################################
#
# Door86 Emulator
#
# set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

project(door86)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()
find_package(fmt CONFIG REQUIRED)


include_directories("${CMAKE_SOURCE_DIR}/deps/wwiv")
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(deps/wwiv/core)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/deps/wwiv)

add_subdirectory(bios)
add_subdirectory(cpu)
add_subdirectory(dis)
add_subdirectory(door86)
add_subdirectory(dos)

