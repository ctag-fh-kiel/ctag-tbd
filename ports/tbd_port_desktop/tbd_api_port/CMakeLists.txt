# configure source files

tbd_platform_apis("${TBD_PLATFORM_OBJ}" VAR api_list)

if (wifi IN_LIST api_list)
    list(APPEND SRCS_FILES src/rest_api.cpp)
    set(extra_libs INTERFACE simple-web-server)
endif()

if (serial IN_LIST api_list)
    tbd_loge("serial api not supported on desktop platform")
    # list(APPEND SRCS_FILES src/serial_api.cpp)
endif()

file(GLOB SRCS_HEADERS
        include/**/*.hpp
)


# component declaration

add_library(tbd_api_port STATIC ${SRCS_FILES} ${SRCS_HEADERS})
target_include_directories(tbd_api_port
        PUBLIC include
        PRIVATE ${CMAKE_SOURCE_DIR}/vendor/Simple-Web-Server
)
target_link_libraries(tbd_api_port
        PUBLIC
            tbd_version
            tbd_system
            tbd_api_common
            tbd_sound_manager
            tbd_presets
)
target_link_libraries(tbd_api_port ${extra_libs})
