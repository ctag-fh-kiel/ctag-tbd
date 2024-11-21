set(VERSION_SRC "${CMAKE_BINARY_DIR}/gen_src/version.cpp")

add_custom_command(
    OUTPUT "${VERSION_SRC}"
    COMMAND ${TBD_TOOL} -p "${TBD_PLATFORM}" project create-build-info
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"    
)

set(SRCS_FILES 
    include/tbd/version.hpp    
    "${VERSION_SRC}"
)

add_library(tbd_version STATIC ${SRCS_FILES})
target_include_directories(tbd_version PUBLIC include)
