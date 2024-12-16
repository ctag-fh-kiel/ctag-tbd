file(GLOB SRCS_FILES 
    src/dummy.cpp
)

add_library(tbd_storage INTERFACE ${SRCS_FILES})
target_include_directories(tbd_storage INTERFACE include)
target_link_libraries(tbd_storage INTERFACE ${TBD_STORAGE_PORT_LIB})
