include(${CMAKE_SOURCE_DIR}/tools/cmake/platforms.cmake)


tbd_platform_audio("${TBD_PLATFORM_OBJ}" VAR codec_type)
tbd_codec_type("${codec_type}")
if ("${_return}" STREQUAL "file_codec")

    list(APPEND extra_srcs src/file_codec.cpp)
endif()


set(tinywav_dir "${CMAKE_SOURCE_DIR}/vendor/tinywav")
file(GLOB SRCS_FILES
        "${tinywav_dir}/tinywav.h"
        "${tinywav_dir}/tinywav.c"
        src/*.hpp
        src/*.cpp
)

add_library(tbd_drivers_port STATIC ${SRCS_FILES} ${extra_srcs})
target_include_directories(tbd_drivers_port PUBLIC include "${tinywav_dir}")
target_link_libraries(tbd_drivers_port
        PUBLIC tbd_drivers_common
        PRIVATE
            tbd_common
)
