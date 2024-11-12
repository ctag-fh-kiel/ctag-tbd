execute_process(COMMAND ${TBD_TOOL} project create-build-info)

set(SRCS_FILES 
    include/tbd/version.hpp 
    ${CMAKE_BINARY_DIR}/gen_src/version.cpp
)

add_library(tbd_version STATIC ${SRCS_FILES})
target_include_directories(tbd_version PUBLIC include)
