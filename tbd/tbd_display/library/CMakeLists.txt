file(GLOB SRCS_FILES 
        include/**/*.hpp
)

add_library(tbd_display INTERFACE ${SRCS_FILES})
target_include_directories(tbd_display INTERFACE include)
target_link_libraries(tbd_display INTERFACE ${TBD_DISPLAY_PORT_LIB})
