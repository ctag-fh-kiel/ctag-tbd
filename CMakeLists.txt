# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# pull in idf specific items, config vars etc.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(LINK_OPTIONS "-Wl,--wrap=sdmmc_read_sectors" APPEND)
idf_build_set_property(LINK_OPTIONS "-Wl,--undefined=__wrap_sdmmc_read_sectors" APPEND)
idf_build_set_property(LINK_OPTIONS "-Wl,--wrap=sdmmc_write_sectors" APPEND)
idf_build_set_property(LINK_OPTIONS "-Wl,--undefined=__wrap_sdmmc_write_sectors" APPEND)

project(ctag-tbd)

# this is global to all!
#idf_build_set_property(COMPILE_OPTIONS -Wno-unused-local-typedefs -ffast-math APPEND) # -ffast-math -fno-finite-math-only https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 APPEND)
idf_build_set_property(COMPILE_DEFINITIONS -DRAPIDJSON_HAS_STDSTRING=1 APPEND)

idf_build_set_property(COMPILE_DEFINITIONS -DN_CVS=240 APPEND)
idf_build_set_property(COMPILE_DEFINITIONS -DN_TRIGS=60 APPEND)
set(TBD_HW "DADA")

# generate include definitions for hw + fw version
execute_process(
        COMMAND bash -c "git describe --tags --abbrev=4"
        OUTPUT_VARIABLE TBD_FW)
string(STRIP ${TBD_FW} TBD_FW)
configure_file(${CMAKE_SOURCE_DIR}/version.hpp.in ${CMAKE_BINARY_DIR}/gen_include/version.hpp @ONLY)
idf_build_set_property(INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/gen_include APPEND)

# Create SD card zip archive and hash for embedded system
set(SD_CARD_ZIP "${CMAKE_BINARY_DIR}/tbd-sd-card.zip")
set(SD_CARD_HASH "${CMAKE_BINARY_DIR}/tbd-sd-card-hash.txt")

# Find xxhash tool
find_program(XXH128SUM xxh128sum REQUIRED)

add_custom_target(sd_card_archive ALL
    COMMAND ${CMAKE_SOURCE_DIR}/create_sd_archive.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${XXH128SUM}
    BYPRODUCTS ${SD_CARD_ZIP} ${SD_CARD_HASH}
    COMMENT "Building SD card archive and hash"
    VERBATIM
)

