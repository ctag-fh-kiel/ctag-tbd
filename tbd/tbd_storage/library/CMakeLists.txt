file(GLOB SRCS_FILES 
    src/dummy.cpp
)

add_library(tbd_storage STATIC ${SRCS_FILES})
target_link_libraries(tbd_storage PUBLIC ${TBD_STORAGE_PORT_LIB})