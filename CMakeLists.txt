# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

# import all TBD macros and functions
include(${CMAKE_SOURCE_DIR}/tools/cmake/project.cmake)

# make sure which platform and which toolchain we are using
tbd_platform_setup()
tbd_toolchain_setup(${TBD_PLATFORM})

# enable platform and toolchain for the entire project
tbd_platform_activate()
tbd_toolchain_activate(${TBD_TOOLCHAIN})


tbd_toolchain_add_defines()

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ctag-tbd)
tbd_storages_setup()



# make the TBD helper tool available to CMake
if(NOT TBD_TOOL)
    find_program(TBD_TOOL NAMES tbd PATHS ${CMAKE_SOURCE_DIR/tools/tbd_tools/bin} NO_CACHE)
endif()

if ("${TBD_TOOLCHAIN}" STREQUAL "desktop")
    add_subdirectory("${CMAKE_SOURCE_DIR}/apps/desktop")
    add_subdirectory("${CMAKE_SOURCE_DIR}/ports/tbd_port_desktop")
endif()


# import libraries
add_subdirectory(vendor)
add_subdirectory(tbd)


# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.

if (TBD_BUILD_FS)

file(GLOB_RECURSE WWW_FILES RELATIVE ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_SOURCE_DIR}/spiffs_image/www/*)
foreach(WWW_FILE ${WWW_FILES})
    list(APPEND GZIP_COMMANDS
            COMMAND gzip -9 ${CMAKE_BINARY_DIR}/spiffs_image/${WWW_FILE} )
endforeach()
if(NOT CONFIG_TBD_PLATFORM_BBA)
    list(APPEND DEL_COMMANDS
            COMMAND rm -f ${CMAKE_BINARY_DIR}/spiffs_image/data/sp/*DrumRack.jsn)
    list(APPEND DEL_COMMANDS
            COMMAND rm -f ${CMAKE_BINARY_DIR}/spiffs_image/dbup/sp/*DrumRack.jsn)
endif()
add_custom_target(copy-files ALL DEPENDS ${CMAKE_SOURCE_DIR}/spiffs_image
    # clean up
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/spiffs_image
    # copy spiffs files to build folder
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_BINARY_DIR}/spiffs_image
    # create gzip encoded www files
    ${GZIP_COMMANDS}
    # and create backup file structure (sometimes when the ESP's power is interrupted during flash writes, the SPIFFS file gets corrupted)
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image/data ${CMAKE_BINARY_DIR}/spiffs_image/dbup
    ${DEL_COMMANDS}
)

littlefs_create_partition_image(storage ${CMAKE_BINARY_DIR}/spiffs_image FLASH_IN_PROJECT DEPENDS copy-files)

endif()

