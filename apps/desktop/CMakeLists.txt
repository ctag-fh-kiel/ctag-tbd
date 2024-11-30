find_package(Boost 1.67.0 COMPONENTS program_options REQUIRED)

file(GLOB SRCS_FILES
    src/*.hpp
    src/*.cpp
)

add_executable(tbd_app_desktop ${SRCS_FILES})
target_include_directories(tbd_app_desktop INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(tbd_app_desktop
    PUBLIC
        tbd_sound_manager
        tbd_config
        tbd_display
        tbd_drivers
        tbd_helpers
        tbd_presets
        tbd_sound_processor
        tbd_system
        tbd_api
        ${Boost_LIBRARIES}
)
