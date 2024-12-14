# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# import all TBD macros and functions
include(${CMAKE_SOURCE_DIR}/tools/cmake/project.cmake)

# make sure which platform and which toolchain we are using
tbd_platform_setup()
tbd_toolchain_setup(${TBD_PLATFORM})

tbd_write_build_info()

# enable platform and toolchain for the entire project
tbd_platform_activate()
tbd_toolchain_activate(${TBD_TOOLCHAIN})
tbd_toolchain_add_defines()

project(ctag-tbd)

tbd_toolchain_add_options()
tbd_storages_activate()

# make the TBD helper tool available to CMake
if(NOT TBD_TOOL)
    find_program(TBD_TOOL NAMES tbd PATHS "${CMAKE_SOURCE_DIR}/tools/bin")
endif()

if ("${TBD_TOOLCHAIN}" STREQUAL "desktop")
    add_subdirectory("${CMAKE_SOURCE_DIR}/apps/${TBD_PLATFORM}")
    add_subdirectory("${CMAKE_SOURCE_DIR}/ports/tbd_port_desktop")
endif()

# import libraries
add_subdirectory(vendor)
add_subdirectory(tbd)
