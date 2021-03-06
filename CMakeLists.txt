# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
        RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 # rapidjson
        RAPIDJSON_HAS_STDSTRING=1 # rapidjson
        N_TRIGS=2
)

add_compile_options(
        -Wno-unused-local-typedefs
        # https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity
        -ffast-math
        #-ffast-math -fno-finite-math-only
)



include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ctag-tbd)


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


