idf_component_register(SRCS gpio.cpp adc.cpp fs.cpp led_rgb.cpp codec.cpp
        INCLUDE_DIRS "."
        REQUIRES ulp driver spiffs)

set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_s_sources ulp/adc.S ulp/spi-bb.S ulp/stack.S)
set(ulp_exp_dep_srcs "adc.cpp")

ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})
