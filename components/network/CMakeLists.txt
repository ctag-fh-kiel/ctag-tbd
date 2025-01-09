idf_component_register(SRCS network.cpp
        INCLUDE_DIRS .
        PRIV_REQUIRES nvs_flash mdns esp_netif esp_wifi)

idf_build_set_property(COMPILE_DEFINITIONS CFG_TUD_NCM_IN_NTB_N=3 APPEND)
idf_build_set_property(COMPILE_DEFINITIONS CFG_TUD_NCM_OUT_NTB_N=2 APPEND)

