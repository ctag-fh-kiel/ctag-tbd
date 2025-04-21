## configure sources ##

# network srcs

tbd_platform_network("${TBD_PLATFORM_OBJ}" VAR has_network)

if (has_network)
    list(APPEND extra_srcs src/network_config.cpp)
endif()

# api srcs

tbd_platform_apis("${TBD_PLATFORM_OBJ}" VAR api_list)

if (shell IN_LIST api_list)
    file(GLOB microshell_srcs
        ${CMAKE_SOURCE_DIR}/vendor/microshell/src/src/*.c
        ${CMAKE_SOURCE_DIR}/vendor/microshell/src/src/commands/*.c
    )

    list(APPEND extra_srcs src/shell.cpp ${microshell_srcs})
    list(APPEND extra_includes
            ${CMAKE_SOURCE_DIR}/vendor/microshell/src
            ${CMAKE_SOURCE_DIR}/vendor/microshell/src/config/arduino
    )
    list(APPEND extra_defines "USH_CONFIG_CUSTOM_FILE=\"tbd_microshell_config.h\"")
endif ()


## library declaration

file(GLOB SRCS_FILES
        include/**/*.hpp
        src/dummy.cpp
)

if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
    set(port_lib __idf_tbd_api_port)
else()
    set(port_lib tbd_api_port)
endif()

add_library(tbd_api STATIC ${SRCS_FILES} ${extra_srcs})
target_include_directories(tbd_api PUBLIC include PRIVATE src ${extra_includes})
target_compile_definitions(tbd_api PRIVATE ${extra_defines})
target_link_libraries(tbd_api PUBLIC tbd_config tbd_api_common ${port_lib})
