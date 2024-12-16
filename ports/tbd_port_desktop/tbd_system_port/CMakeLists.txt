file(GLOB SRCS_FILES
    include/tbd/**/*.hpp
    src/dummy.cpp
)

add_library(tbd_system_port INTERFACE ${SRCS_FILES})
target_include_directories(tbd_system_port
        INTERFACE include

)
target_link_libraries(tbd_system_port
        INTERFACE tbd_system_common

)
