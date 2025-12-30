idf_component_register(
    SRCS "perf_monitor.cpp"
    INCLUDE_DIRS "include"
    REQUIRES freertos esp_timer
    PRIV_REQUIRES esp_hw_support
)

