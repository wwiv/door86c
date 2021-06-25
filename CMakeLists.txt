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
endif()
include_directories(${CMAKE_SOURCE_DIR})


add_subdirectory(cpu)
add_subdirectory(dis)
