file(GLOB SRCS_FILES 
        include/tbd/drivers/**/*.hpp
)

add_library(tbd_drivers_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_drivers_common INTERFACE include)
target_link_libraries(tbd_drivers_common INTERFACE tbd_system)
