file(GLOB SRCS_FILES 
        include/tbd/drivers/*.hpp
)

add_library(tbd_drivers STATIC ${SRCS_FILES})
target_include_directories(tbd_drivers PUBLIC include)
target_link_libraries(tbd_drivers PUBLIC __idf_tbd_drivers_port)
