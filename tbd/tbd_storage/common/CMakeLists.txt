file(GLOB SRCS_FILES 
        include/tbd/storage/*.hpp
)

add_library(tbd_storage_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_storage_common INTERFACE include)
