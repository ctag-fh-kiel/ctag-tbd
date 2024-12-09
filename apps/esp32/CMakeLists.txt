include(${TBD_CMAKE_DIR}/build_config/platforms.cmake)

tbd_platform_display("${TBD_PLATFORM_OBJ}" VAR has_display)
if (has_display)
    set(display_lib tbd_display)
endif()

file(GLOB SRC_FILES *.cpp *.hpp)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES app_update esp_http_server
)
target_link_libraries(${COMPONENT_LIB} PUBLIC 
        tbd_version 
        tbd_sound_manager
        tbd_presets
        tbd_drivers
        ${display_lib}
        tbd_calibration
        tbd_api
)
target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
)
