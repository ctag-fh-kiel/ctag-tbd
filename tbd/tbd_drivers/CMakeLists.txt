set(esp_dsp __idf_espressif_esp-dsp)
set(esp_driver __idf_driver)
set(esp_tinyusb __idf_espressif__esp_tinyusb)
set(esp_system __idf_esp_system)
set(esp_littlefs __idf_joltwallet__littlefs)
set(esp_led_strip __idf_espressif__led_strip)

if(CONFIG_TBD_PLATFORM_MK2)
    add_library(tbd_drivers fs.cpp led_rgb.cpp codec.cpp mk2.cpp ssd1306.c ssd1306_i2c.c)
    target_link_libraries(tbd_drivers PRIVATE ${esp_driver})
elseif (CONFIG_TBD_PLATFORM_BBA)
    if(CONFIG_TBD_BBA_CODEC_ES8388)
        set(codec_src es8388.cpp)
    else()
        set(codec_src aic3254.cpp)
    endif()

    message("########################## HELLO")

    add_library(tbd_drivers fs.cpp led_rgb_bba.cpp codec_bba.cpp ${codec_src} midiuart.cpp tusbmidi.cpp ssd1306.c ssd1306_i2c.c)
    target_link_libraries(tbd_drivers PUBLIC 
        ${esp_dsp} 
        ${esp_driver} 
        ${esp_tinyusb} 
        ${esp_system} 
        ${esp_littlefs} 
        ${esp_led_strip}
    )
else()
    add_library(tbd_drivers gpio.cpp adc.cpp fs.cpp led_rgb.cpp codec.cpp ssd1306.c ssd1306_i2c.c)
    target_link_libraries(tbd_drivers PRIVATE ulp ${esp_driver})

    set(ulp_app_name ulp_drivers)

    if(CONFIG_TBD_PLATFORM_STR)
        set(ulp_s_sources ulp/mcp3208.S)
    else()
        set(ulp_s_sources ulp/adc.S)
    endif()

    set(ulp_exp_dep_srcs "adc.cpp")

    ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})
endif()

target_include_directories(tbd_drivers PUBLIC .)
