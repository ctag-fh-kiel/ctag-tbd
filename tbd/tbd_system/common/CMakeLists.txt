file(GLOB SRCS_FILES 
    include/tbd/**/*.hpp    
)

add_library(tbd_system_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_system_common INTERFACE include)
