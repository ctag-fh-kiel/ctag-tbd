file(GLOB SRCS_FILES 
    include/tbd/*.hpp
    src/*.hpp
    src/*.cpp
)

idf_component_register(
    SRCS ${SRCS_FILES}
    REQUIRES tbd_system nvs_flash esp_netif esp_wifi mdns
)

target_link_libraries(${COMPONENT_LIB} PUBLIC tbd_network)
