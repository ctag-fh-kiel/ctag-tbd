# configure source files
find_package(Boost 1.67.0 COMPONENTS system REQUIRED)
tbd_platform_api_types("${TBD_PLATFORM_OBJ}" VAR api_list)



if (rest IN_LIST api_list)
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
        PRIVATE ${CMAKE_SOURCE_DIR}/vendor/Simple-Web-Server ${Boost_INCLUDE_DIR}
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
