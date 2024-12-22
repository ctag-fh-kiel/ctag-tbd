if(CONFIG_TBD_PLATFORM_MK2)
    idf_component_register(SRCS fs.cpp led_rgb.cpp codec.cpp mk2.cpp ssd1306.c ssd1306_i2c.c
            INCLUDE_DIRS "."
            REQUIRES driver )
elseif (CONFIG_TBD_PLATFORM_BBA)


    idf_component_register(SRCS fs.cpp led_rgb_bba.cpp codec_bba.cpp ${codec_src} midiuart.cpp tusbmidi.cpp ssd1306.c ssd1306_i2c.c ssd1306_spi.c
            INCLUDE_DIRS "."
            REQUIRES driver  esp-dsp esp-tinyusb)
else()
    idf_component_register(SRCS gpio.cpp adc.cpp fs.cpp led_rgb.cpp codec.cpp ssd1306.c ssd1306_spi.c ssd1306_i2c.c
            INCLUDE_DIRS "."
            REQUIRES ulp driver )

    set(ulp_app_name ulp_${COMPONENT_NAME})

    if(CONFIG_TBD_PLATFORM_STR)
        set(ulp_s_sources ulp/mcp3208.S)
    else()
        set(ulp_s_sources ulp/adc.S)
    endif()

    set(ulp_exp_dep_srcs "adc.cpp")

    ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})
endif()


