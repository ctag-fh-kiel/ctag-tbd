file(GLOB SRCS_FILES 
    include/tbd/storage/*.hpp
    src/*.hpp
    src/*.cpp
)

idf_component_register(
    SRCS ${SRCS_FILES}
    INCLUDE_DIRS include
    REQUIRES 
        spi_flash
        esp_netif 
        esp_wifi 
        mdns
)

target_link_libraries(${COMPONENT_LIB} PUBLIC tbd_system tbd_storage_common)
