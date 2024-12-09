set(TBD_CMAKE_DIR "${CMAKE_SOURCE_DIR}/tools/cmake" CACHE STRING "" FORCE)
mark_as_advanced(TBD_CMAKE_DIR)

include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/build_config/platforms.cmake)
include(${TBD_CMAKE_DIR}/build_config/toolchains.cmake)
include(${TBD_CMAKE_DIR}/build_config/build_info.cmake)
include(${TBD_CMAKE_DIR}/storages.cmake)
