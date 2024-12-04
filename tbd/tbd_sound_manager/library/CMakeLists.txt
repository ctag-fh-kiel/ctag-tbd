if ("${TBD_TOOLCHAIN}" STREQUAL "desktop")
    set(extra_libs tbd_sound_manager_port)
endif()

file(GLOB SRCS_FILES
    include/tbd/**/*.hpp
    src/*.cpp
    src/*.hpp
)

add_library(tbd_sound_manager STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_manager
    PUBLIC include
)
target_link_libraries(tbd_sound_manager 
    PUBLIC
        tbd_common
        tbd_sound_registry
        tbd_sound_manager_common
        tbd_drivers
        ${extra_libs}
    PRIVATE
        tbd_system
        tbd_storage

        tbd_presets
        tbd_config
        tbd_calibration
)

target_compile_options(tbd_sound_manager PRIVATE
        -Wno-unused-local-typedefs
)
