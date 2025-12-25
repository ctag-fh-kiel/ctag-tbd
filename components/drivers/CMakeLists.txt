idf_component_register(SRCS "fs.cpp" "led_rgb_bba.cpp" "codec_bba.cpp" "tusb.cpp" "rp2350_spi_stream.cpp"
        INCLUDE_DIRS "."
        REQUIRES ableton_link driver)