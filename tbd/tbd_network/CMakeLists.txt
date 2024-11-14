file(GLOB SRCS_FILES 
        include/tbd/*.hpp
)

add_library(tbd_network INTERFACE ${SRCS_FILES})
target_include_directories(tbd_network INTERFACE include)
