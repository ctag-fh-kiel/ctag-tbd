idf_component_register(SRCS network.cpp
        INCLUDE_DIRS . $ENV{IDF_PATH}/examples/common_components/protocol_examples_common/include
        PRIV_REQUIRES nvs_flash mdns esp_netif)

