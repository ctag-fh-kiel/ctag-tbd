file(GLOB SRCS_FILES 
        include/tbd/**/*.hpp
)

add_library(tbd_display_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_display_common INTERFACE include)
