set(DRIVER_SRCS
    "fs.cpp" "led_rgb_bba.cpp"
    "codec_bba.cpp" "tusb.cpp"
)

if(CONFIG_TBD_USE_RP2350)
    list(APPEND DRIVER_SRCS "rp2350_spi_stream.cpp" "pico_reset.cpp" "picoboot3_master.cpp" "pico_firmware_update.cpp")
endif()

if(CONFIG_TBD_USE_SD_CARD)
    list(APPEND DRIVER_SRCS "custom_sdmmc_cmd.cpp")
endif()

set(DRIVER_REQUIRES ableton_link driver littlefs esp_timer)

idf_component_register(SRCS ${DRIVER_SRCS}
        INCLUDE_DIRS "."
        REQUIRES ${DRIVER_REQUIRES}
        PRIV_REQUIRES esp_driver_i2s esp_driver_i2c)

# SDMMC DMA buffer wrapper: redirect sdmmc_read/write_sectors through our
# custom_sdmmc_cmd.cpp which uses a pre-allocated PSRAM DMA buffer instead
# of per-call internal-RAM DMA allocation (prevents OOM under memory pressure).
# Must use target_link_libraries INTERFACE here — idf_build_set_property in the
# root CMakeLists.txt after project() is silently ignored by ESP-IDF's build system.
if(CONFIG_TBD_USE_SD_CARD)
    target_link_libraries(${COMPONENT_LIB} INTERFACE
        "-Wl,--wrap=sdmmc_read_sectors"
        "-Wl,--undefined=__wrap_sdmmc_read_sectors"
        "-Wl,--wrap=sdmmc_write_sectors"
        "-Wl,--undefined=__wrap_sdmmc_write_sectors"
    )
endif()
