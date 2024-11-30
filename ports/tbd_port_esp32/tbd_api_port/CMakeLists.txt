# configure source files

tbd_platform_apis("${TBD_PLATFORM_OBJ}" VAR api_list)

if (wifi IN_LIST api_list)
    list(APPEND SRCS_FILES src/rest_api.cpp src/network.cpp)
    list(APPEND extra_libs
            json
            esp_wifi
            esp_netif
            nvs_flash
            esp_http_server
            mdns
    )
endif()

if (serial IN_LIST api_list)
    list(APPEND SRCS_FILES src/serial_api.cpp)
endif()

file(GLOB SRCS_HEADERS
        include/**/*.hpp
)


# component declaration

idf_component_register(
    SRCS ${SRCS_FILES} ${SRCS_HEADERS}
    INCLUDE_DIRS include
    REQUIRES ${extra_libs}
)

target_link_libraries(${COMPONENT_LIB}
        PUBLIC
            tbd_version
            tbd_system
            tbd_api_common
            tbd_sound_manager
            tbd_presets
)
