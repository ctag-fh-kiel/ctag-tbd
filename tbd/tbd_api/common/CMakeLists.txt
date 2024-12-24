## configure flags ##

# network options

tbd_platform_network("${TBD_PLATFORM_OBJ}" VAR has_network)

if (has_network)
    set(network_flag TBD_NETWORK=1)
else()
    set(network_flag TBD_NETWORK=0)
endif()


# api options

tbd_platform_api_types("${TBD_PLATFORM_OBJ}" VAR api_list)

set(rest_flag TBD_API_REST=0)
set(serial_flag TBD_API_SERIAL=0)
set(shell_flag TBD_API_SERIAL=0)

if (rest IN_LIST api_list)
    set(rest_flag TBD_API_REST=1)
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
            ${rest_flag}
            ${network_flag}
            ${serial_flag}
            ${shell_flag}
)
target_link_libraries(tbd_api_common INTERFACE tbd_config)
