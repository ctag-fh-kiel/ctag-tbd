file(GLOB SRCS_FILES 
        include/drivers/*.hpp
)

add_library(tbd_drivers INTERFACE ${SRCS_FILES})
target_include_directories(tbd_drivers INTERFACE include)
