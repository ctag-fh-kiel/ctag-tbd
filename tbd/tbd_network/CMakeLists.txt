file(GLOB SRCS_FILES 
        include/tbd/*.hpp
        src/*.hpp
        src/*.cpp
)

add_library(tbd_network STATIC ${SRCS_FILES})
target_include_directories(tbd_network PUBLIC include)
target_link_libraries(tbd_network PUBLIC tbd_config)

