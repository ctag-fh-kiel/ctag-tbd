idf_component_register(SRCS gpio.cpp adc.cpp fs.cpp led_rgb.cpp codec.cpp display.cpp
        INCLUDE_DIRS "."
        REQUIRES ulp driver spiffs tarablessd1306)

set(ulp_app_name ulp_${COMPONENT_NAME})

if(CONFIG_TBD_PLATFORM_STR)
    set(ulp_s_sources ulp/mcp3208.S)
else()
    set(ulp_s_sources ulp/adc.S)
endif()

set(ulp_exp_dep_srcs "adc.cpp")

ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})

# number of CV ins
if(CONFIG_TBD_PLATFORM_STR)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE N_CVS=8)
else()
    target_compile_definitions(${COMPONENT_LIB} PRIVATE N_CVS=4)
endif()
