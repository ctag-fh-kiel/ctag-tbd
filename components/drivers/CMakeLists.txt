idf_component_register(SRCS "fs.cpp" "led_rgb_bba.cpp" "codec_bba.cpp" "tusb.cpp" "ssd1306.c" "ssd1306_i2c.c" "ssd1306_spi.c" "rp2350_spi_stream.cpp"
        INCLUDE_DIRS "."
        REQUIRES ableton_link driver)