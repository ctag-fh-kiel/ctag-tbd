file(GLOB SRCS_FILES 
    include/tbd/**/*.hpp    
)

add_library(tbd_system INTERFACE ${SRCS_FILES})
target_include_directories(tbd_system INTERFACE include)
target_link_libraries(tbd_system INTERFACE tbd_system_common ${TBD_SYSTEM_PORT_LIB})
