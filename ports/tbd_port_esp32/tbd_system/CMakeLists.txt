set(PORTS_COMMON_DIR ${CMAKE_SOURCE_DIR}/ports/common)
set(PORTS_SYSTEM_COMMON_INCLUDE ${PORTS_COMMON_DIR}/tbd_system/include)

file(GLOB SRCS_FILES 
    include/tbd/*.hpp
    ${PORTS_SYSTEM_COMMON_INCLUDE}/tbd/*.hpp
)

idf_component_register(
    SRCS ${SRCS_FILES} 
    INCLUDE_DIRS include ${PORTS_SYSTEM_COMMON_INCLUDE}
    REQUIRES log heap
)
