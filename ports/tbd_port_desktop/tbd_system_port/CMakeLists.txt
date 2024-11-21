file(GLOB SRCS_FILES 
    include/tbd/**/*.hpp
    src/dummy.cpp
)

add_library(tbd_system_port STATIC ${SRCS_FILES})
target_include_directories(tbd_system_port PUBLIC include)
target_link_libraries(tbd_system_port PUBLIC tbd_system_common)
