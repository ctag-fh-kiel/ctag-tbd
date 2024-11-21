# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    src/*.cpp
    src/*.hpp
)

add_library(tbd_display_port STATIC ${SRCS_FILES})
target_link_libraries(tbd_display_port PUBLIC tbd_version tbd_display_common)
