if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
    set(port_lib __idf_tbd_api_port)
else()
    set(port_lib tbd_api_port)
endif()


file(GLOB SRCS_FILES
        include/**/*.hpp
        src/*.hpp
        src/*.cpp
)

add_library(tbd_api STATIC ${SRCS_FILES})
target_include_directories(tbd_api PUBLIC include)
target_link_libraries(tbd_api PUBLIC tbd_config tbd_api_common ${port_lib})
