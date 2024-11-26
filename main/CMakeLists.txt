file(GLOB SRC_FILES *.cpp *.hpp)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES 
                tbd_network_port 
                esp_http_server 
                esp_partition 
                nvs_flash 
                app_update 
                json
                esp-dsp
)
target_link_libraries(${COMPONENT_LIB} PUBLIC 
        tbd_version 
        tbd_sound_manager
        tbd_presets
        tbd_drivers
        tbd_display
        tbd_network
        tbd_calibration
)
target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
)
