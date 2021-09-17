# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pull in idf specific items, config vars etc.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ctag-tbd)

# this is global to all!
#idf_build_set_property(COMPILE_OPTIONS -Wno-unused-local-typedefs -ffast-math APPEND) # -ffast-math -fno-finite-math-only https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 APPEND)
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_HAS_STDSTRING=1 APPEND)

if(CONFIG_TBD_PLATFORM_STR)
    message("Configuring for Strämpler!")
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=8 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
    set(TBD_HW "Strampler")
elseif(CONFIG_TBD_PLATFORM_MK2)
    message("Configuring for MK2!")
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=22 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=12 APPEND)
    set(TBD_HW "mk2")
elseif(CONFIG_TBD_PLATFORM_V2)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
    set(TBD_HW "V2")
elseif(CONFIG_TBD_PLATFORM_V1)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
    set(TBD_HW "V1")
elseif(CONFIG_TBD_PLATFORM_AEM)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
    set(TBD_HW "AEM")
endif()

# generate include definitions for hw + fw version
execute_process(
        COMMAND bash -c "git describe --tags --abbrev=4"
        OUTPUT_VARIABLE TBD_FW)
string(STRIP ${TBD_FW} TBD_FW)
configure_file(${CMAKE_SOURCE_DIR}/version.hpp.in ${CMAKE_BINARY_DIR}/gen_include/version.hpp @ONLY)
idf_build_set_property(INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/gen_include APPEND)

# Create a SPIFFS image from the contents of the 'spiffs_image' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash'.
file(GLOB_RECURSE WWW_FILES RELATIVE ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_SOURCE_DIR}/spiffs_image/www/*)
foreach(WWW_FILE ${WWW_FILES})
    list(APPEND GZIP_COMMANDS
            COMMAND gzip -9 ${CMAKE_BINARY_DIR}/spiffs_image/${WWW_FILE} )
endforeach()
add_custom_target(copy-files ALL DEPENDS ${CMAKE_SOURCE_DIR}/spiffs_image
        # clean up
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/spiffs_image
        # copy spiffs files to build folder
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image ${CMAKE_BINARY_DIR}/spiffs_image
        # create gzip encoded www files
        ${GZIP_COMMANDS}
        # and create backup file structure (sometimes when the ESP's power is interrupted during flash writes, the SPIFFS file gets corrupted)
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/spiffs_image/data ${CMAKE_BINARY_DIR}/spiffs_image/dbup
        )

spiffs_create_partition_image(storage ${CMAKE_BINARY_DIR}/spiffs_image FLASH_IN_PROJECT DEPENDS copy-files)


