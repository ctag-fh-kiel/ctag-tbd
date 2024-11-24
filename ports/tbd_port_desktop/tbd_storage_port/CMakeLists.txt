file(GLOB SRCS_FILES 
    include/**/*.hpp
    src/*.hpp
    src/*.cpp
)

add_library(tbd_storage_port STATIC ${SRCS_FILES})
target_include_directories(tbd_storage_port PUBLIC include)
target_link_libraries(tbd_storage_port PUBLIC tbd_system tbd_storage_common)
