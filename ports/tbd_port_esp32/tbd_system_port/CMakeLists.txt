file(GLOB SRCS_FILES 
    include/tbd/*.hpp
)

idf_component_register(
    SRCS ${SRCS_FILES} 
    INCLUDE_DIRS include 
    REQUIRES log heap esp_system
)
target_link_libraries(${COMPONENT_LIB} INTERFACE tbd_system)
