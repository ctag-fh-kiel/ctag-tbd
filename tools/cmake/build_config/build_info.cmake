include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/build_config/toolchains.cmake)
include(${TBD_CMAKE_DIR}/build_config/platforms.cmake)

# @brief places `build_info.txt` file in build root
#
# Build info contains basic information about build, to be read by helpers
# such as bash scripts, but has no effect on build.
#
function(tbd_write_build_info)
    configure_file(${TBD_CMAKE_DIR}/build_config/build_info.txt.in ${CMAKE_BINARY_DIR}/build_info.txt @ONLY)
endfunction()