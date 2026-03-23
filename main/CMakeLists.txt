file(GLOB SRC_FILES *.cpp *.hpp)

# Macro/preset system is a dadamachines TBD-16 addition, requires SD card storage.
# Excludes: Macro*, SynthDefinition*, TrackDefinition*
if(NOT CONFIG_TBD_USE_SD_CARD)
    list(FILTER SRC_FILES EXCLUDE REGEX "Macro.*")
    list(FILTER SRC_FILES EXCLUDE REGEX "SynthDefinition.*")
    list(FILTER SRC_FILES EXCLUDE REGEX "TrackDefinition.*")
endif()

set(MAIN_REQUIRES esp_psram network ableton_link spi_flash esp_http_server drivers rapidjson app_update ctagSoundProcessor esp_timer fatfs)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES ${MAIN_REQUIRES})

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
        )

