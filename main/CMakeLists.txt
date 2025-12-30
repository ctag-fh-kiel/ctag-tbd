file(GLOB SRC_FILES *.cpp *.hpp)


idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES esp_psram network ableton_link spi_flash esp_http_server drivers rapidjson app_update ctagSoundProcessor
PRIV_REQUIRES instrumentation)


target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
        )

