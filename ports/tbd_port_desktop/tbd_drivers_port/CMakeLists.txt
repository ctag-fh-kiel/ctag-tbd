file(GLOB SRCS_FILES 
    src/*.hpp
    src/*.cpp
)

add_library(tbd_drivers_port STATIC ${SRCS_FILES})
target_include_directories(tbd_drivers_port PUBLIC include)
target_link_libraries(tbd_drivers_port PUBLIC tbd_drivers_common)
