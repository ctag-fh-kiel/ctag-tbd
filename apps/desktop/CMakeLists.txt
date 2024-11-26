
file(GLOB SRCS_FILES
    src/*.hpp
    src/*.cpp
)

add_executable(tbd_app_desktop ${SRCS_FILES})
target_link_libraries(tbd_app_desktop
    PRIVATE 
        tbd_config
        tbd_display
        tbd_drivers
        tbd_helpers
        tbd_network
        tbd_presets
        tbd_sound_processor
        tbd_system

)
