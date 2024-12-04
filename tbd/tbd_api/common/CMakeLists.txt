## configure flags ##

# network options

tbd_platform_network("${TBD_PLATFORM_OBJ}" VAR has_network)

if (has_network)
    set(network_flag TBD_NETWORK=1)
else()
    set(network_flag TBD_NETWORK=0)
endif()


# api options

tbd_platform_apis("${TBD_PLATFORM_OBJ}" VAR api_list)

set(wifi_flag TBD_API_WIFI=0)
set(serial_flag TBD_API_SERIAL=0)
set(shell_flag TBD_API_SERIAL=0)

# the network module is only available on embedded devices if the 'wifi' api is present
if (wifi IN_LIST api_list AND "${TBD_TOOLCHAIN}" STREQUAL "esp32")
    set(wifi_flag TBD_API_WIFI=1)

elseif (wifi IN_LIST api_list)
    set(wifi_flag TBD_API_WIFI=1)
endif()

if (serial IN_LIST api_list)
    set(serial_flag TBD_API_SERIAL=1)
endif()

if (shell IN_LIST api_list)
    set(shell_flag TBD_API_SHELL=1)
endif()

file(GLOB SRCS_FILES
        include/**/*.hpp
        src/*.hpp
        src/*.cpp
)


## interface declaration ##

add_library(tbd_api_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_api_common INTERFACE include)
target_compile_definitions(tbd_api_common
        INTERFACE
            ${wifi_flag}
            ${network_flag}
            ${serial_flag}
            ${shell_flag}
)
target_link_libraries(tbd_api_common INTERFACE tbd_config)
