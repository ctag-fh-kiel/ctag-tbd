## configure srcs ##

# network

tbd_platform_network("${TBD_PLATFORM_OBJ}" VAR has_network)

if (has_network)
    list(APPEND extra_srcs src/network.cpp)
    list(APPEND extra_components esp_wifi esp_netif nvs_flash mdns)
endif()


# APIs

tbd_platform_apis("${TBD_PLATFORM_OBJ}" VAR api_list)

if (wifi IN_LIST api_list)
    list(APPEND extra_srcs src/rest_api.cpp)
    list(APPEND extra_components json esp_http_server)
endif()

if (serial IN_LIST api_list)
    list(APPEND extra_srcs src/serial_api.cpp)
endif()


## component declaration ##

file(GLOB SRCS_FILES
        include/**/*.hpp
        src/dummy.cpp
)

idf_component_register(
    SRCS ${SRCS_FILES} ${extra_srcs}
    INCLUDE_DIRS include
    REQUIRES ${extra_components}
)

target_link_libraries(${COMPONENT_LIB}
        PUBLIC
            tbd_version
            tbd_system
            tbd_api_common
            tbd_sound_manager
            tbd_presets
            tbd_calibration
)
