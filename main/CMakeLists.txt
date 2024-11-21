file(GLOB SRC_FILES *.cpp *.hpp)


# if not CONFIG_TBD_PLATFORM_BBA remove Midi.* files from SRC_FILES
if(NOT CONFIG_TBD_PLATFORM_BBA)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Midi.cpp)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Midi.hpp)
endif()


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
)
target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
)
