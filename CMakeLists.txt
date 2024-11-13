# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/ports/tbd_port_esp32)

project(ctag-tbd)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIXME: move to rapidjson lib
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 APPEND)
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_HAS_STDSTRING=1 APPEND)


idf_build_set_property(COMPILE_OPTIONS -Wno-unused-local-typedefs -ffast-math APPEND) # -ffast-math -fno-finite-math-only https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity
# FIXME: harmonize ESP compiler flags properly
add_compile_options(-ffast-math -ffunction-sections -fdata-sections -gdwarf-4 -ggdb -mdisable-hardware-atomics -O2 -fstrict-volatile-bitfields -fno-jump-tables -fno-tree-switch-conversion -Wno-unused-local-typedefs -std=gnu++2b -fno-exceptions -fno-rtti)

if(CONFIG_TBD_PLATFORM_STR)
    add_compile_definitions(N_CVS=8)
    add_compile_definitions(N_TRIGS=2)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=8 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

    set(TBD_PORT_LIB tbd_port_esp32)
    set(TBD_HW "Strampler")
elseif(CONFIG_TBD_PLATFORM_MK2)
    add_compile_definitions(N_CVS=22)
    add_compile_definitions(N_TRIGS=12)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=22 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=12 APPEND)

    set(TBD_PORT_LIB tbd_port_esp32)
    set(TBD_HW "mk2")
elseif(CONFIG_TBD_PLATFORM_V2)
    add_compile_definitions(N_CVS=4)
    add_compile_definitions(N_TRIGS=2)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

    set(TBD_PORT_LIB tbd_port_esp32)
    set(TBD_HW "V2")
elseif(CONFIG_TBD_PLATFORM_V1)
    add_compile_definitions(N_CVS=4)
    add_compile_definitions(N_TRIGS=2)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)

    set(TBD_PORT_LIB tbd_port_esp32)
    set(TBD_HW "V1")
elseif(CONFIG_TBD_PLATFORM_AEM)
    add_compile_definitions(N_CVS=4)
    add_compile_definitions(N_TRIGS=2)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=4 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=2 APPEND)
    set(TBD_PORT_LIB tbd_port_esp32)
    set(TBD_HW "AEM")
elseif(CONFIG_TBD_PLATFORM_BBA)
    add_compile_definitions(N_CVS=90)
    add_compile_definitions(N_TRIGS=40)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=90 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=40 APPEND)

    set(TBD_PORT_LIB GLOBAL tbd_port_esp32)
    set(TBD_HW "BBA")
elseif(CONFIG_TBD_PLATFORM_DESKTOP)
    add_compile_definitions(N_CVS=90)
    add_compile_definitions(N_TRIGS=40)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=90 APPEND)
    idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=40 APPEND)
    
    set(TBD_PORT_LIB tbd_port_desktop)
    set(TBD_HW "desktop")
else()
    message("no platform selected")
endif()

if(NOT TBD_TOOL)
    find_program(TBD_TOOL NAMES tbd PATHS ${CMAKE_SOURCE_DIR/tools/tbd_tools/bin} NO_CACHE)
endif()

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