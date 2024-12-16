tbd_platform_audio("${TBD_PLATFORM_OBJ}")
tbd_codec_type("${_return}")
if ("${_return}" STREQUAL "rtaudio")
    set(external_libs rtaudio)
endif()

file(GLOB SRCS_FILES
    include/**/*.hpp
    src/*.hpp
    src/*.cpp
)

add_library(tbd_sound_manager_port INTERFACE ${SRCS_FILES})
target_include_directories(tbd_sound_manager_port
        INTERFACE
            include
            "${CMAKE_SOURCE_DIR}/vendor/rtaudio"
)
target_link_libraries(tbd_sound_manager_port
        INTERFACE
            tbd_system
            tbd_sound_manager_common
            tbd_common
            tbd_drivers
            ${external_libs}
)
