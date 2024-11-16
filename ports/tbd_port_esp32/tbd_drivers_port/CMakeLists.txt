function(_tbd_get_driver_build_args)
    set(options FILE_SYSTEM DISPLAY)
    set(single_value_args CV_INPUT AUDIO_OUTPUT INDICATOR)
    set(multi_value_args "")
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${single_value_args}" "${multi_value_args}")

    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    set(libs driver)

    # # gather CV input sources

    if (DEFINED arg_CV_INPUT)
        if (arg_CV_INPUT STREQUAL adc)
            set(cv_input_srcs adc.cpp)
            set(triggers_srcs gpio.cpp)
            set(cv_ulp_srcs ulp/adc.S adc.cpp)
            list(APPEND includes triggers)
            list(APPEND libs ulp)
        elseif (arg_CV_INPUT STREQUAL mcp3208)
            set(cv_input_srcs adc.cpp)
            set(cv_ulp_srcs ulp/mcp3208.S adc.cpp)
            set(triggers_srcs gpio.cpp)
            list(APPEND includes triggers)
            list(APPEND libs ulp)
        elseif (arg_CV_INPUT STREQUAL stm32)
            set(cv_input_srcs mk2.cpp)
            # no GPIO triggers
            # no ULP
        elseif (arg_CV_INPUT STREQUAL midi)
            list(APPEND libs esp-tinyusb)
            set(cv_input_srcs  midiuart.cpp tusbmidi.cpp)
            # no GPIO triggers
            # no ULP
        else ()
            message("invalid CV input ${arg_CV_INPUT}")
        endif ()   

        list(APPEND includes cv_inputs)
        list(TRANSFORM cv_input_srcs PREPEND cv_inputs/)
        list(TRANSFORM cv_ulp_srcs PREPEND cv_inputs/)
        list(TRANSFORM triggers_srcs PREPEND triggers/)
        list(APPEND srcs ${cv_input_srcs} ${triggers_srcs})
    else ()
        message("no CV input specified")
    endif ()  

    # gather audio codec sources

    if (DEFINED arg_AUDIO_OUTPUT)
        if (arg_AUDIO_OUTPUT STREQUAL wm8xxx)
            set(codec_srcs codec.cpp)        
        elseif(arg_AUDIO_OUTPUT STREQUAL aic3254)
            set(codec_srcs codec_bba.cpp aic3254.cpp)
        elseif(arg_AUDIO_OUTPUT STREQUAL es8388)
            set(codec_srcs codec_bba.cpp es8388.cpp)
        else()
            message("invalid sound codec ${arg_AUDIO_OUTPUT}")
        endif()           

        list(APPEND includes codecs)
        list(TRANSFORM codec_srcs PREPEND codecs/)
        list(APPEND srcs ${codec_srcs})
    else()
        message("no sound codec specified")
    endif()  

    # gather indicator

    if (DEFINED arg_INDICATOR)
        if (arg_INDICATOR STREQUAL plain)
            set(indicator_srcs led_rgb.cpp)        
        elseif(arg_INDICATOR STREQUAL neopixel)
            set(indicator_srcs led_rgb_bba.cpp)  
        else()
            message("invalid indicator type ${arg_INDICATOR}")
        endif()           

        list(APPEND includes indicator)
        list(TRANSFORM indicator_srcs PREPEND indicator/)
        list(APPEND srcs ${indicator_srcs})
    endif() 

    if (arg_FILE_SYSTEM)
        list(APPEND includes file_system)
        set(file_system_srcs fs.hpp fs.cpp)
        list(TRANSFORM file_system_srcs PREPEND file_system/)
        list(APPEND srcs ${file_system_srcs})
    endif()

    if (arg_DISPLAY)
        list(APPEND includes display)
        set(display_srcs ssd1306.c ssd1306_i2c.c)
        list(TRANSFORM display_srcs PREPEND display/)
        list(APPEND srcs ${display_srcs})
    endif()

    set(driver_srcs ${srcs} PARENT_SCOPE)
    set(driver_includes ${includes} PARENT_SCOPE)
    set(driver_libs ${libs} PARENT_SCOPE)
    set(driver_ulp_srcs ${cv_ulp_srcs} PARENT_SCOPE)
endfunction()

function(tbd_driver_sources var_name)
    _tbd_get_driver_build_args(${ARGN})
    set(${var_name} ${driver_srcs} PARENT_SCOPE)
endfunction()

function(tbd_driver_includes var_name)
    _tbd_get_driver_build_args(${ARGN})
    set(${var_name} ${driver_includes} PARENT_SCOPE)
endfunction()

function(tbd_driver_libs var_name)
    _tbd_get_driver_build_args(${ARGN})
    set(${var_name} ${driver_libs} PARENT_SCOPE)
endfunction()

function(tbd_driver_embed_ulp)
    _tbd_get_driver_build_args(${ARGV})
    if (driver_ulp_srcs)
        set(ulp_app_name ulp_${COMPONENT_NAME})
        ulp_embed_binary(${ulp_app_name} ${driver_ulp_srcs})
    endif()
endfunction()



if (CONFIG_TBD_PLATFORM_V1)
    set(DRIVER_PARAMS 
        CV_INPUT adc 
        AUDIO_OUTPUT wm8xxx 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR plain 
    )

elseif (CONFIG_TBD_PLATFORM_V2)
    set(DRIVER_PARAMS 
        CV_INPUT adc 
        AUDIO_OUTPUT wm8xxx 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR plain 
    )

elseif (CONFIG_TBD_PLATFORM_STR)
    set(DRIVER_PARAMS 
        CV_INPUT mcp3208 
        AUDIO_OUTPUT wm8xxx 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR plain 
    )

elseif (CONFIG_TBD_PLATFORM_AEM)
    set(DRIVER_PARAMS 
        CV_INPUT adc 
        AUDIO_OUTPUT wm8xxx 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR plain 
    )

elseif (CONFIG_TBD_PLATFORM_MK2)
    set(DRIVER_PARAMS 
        CV_INPUT stm32 
        AUDIO_OUTPUT wm8xxx 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR plain 
    )

elseif (CONFIG_TBD_PLATFORM_BBA)
    if (CONFIG_TBD_BBA_CODEC_ES8388)
        set(audio_output es8388)
    else()
        set(audio_output aic3254)
    endif()

    set(DRIVER_PARAMS
        CV_INPUT midi 
        AUDIO_OUTPUT ${audio_output} 
        FILE_SYSTEM 
        DISPLAY 
        INDICATOR neopixel
    )

else()
    message("platform unknown or not suported by TBD esp32 port")
endif()

tbd_driver_sources(SRCS_FILES ${DRIVER_PARAMS})
tbd_driver_includes(INCLUDE_DIRS ${DRIVER_PARAMS})
tbd_driver_libs(LIBS ${DRIVER_PARAMS})

message(")))))))))))))))))))))))) ${INCLUDE_DIRS}")

idf_component_register(
    SRCS ${SRCS_FILES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES ${LIBS}
)

tbd_driver_embed_ulp(${DRIVER_PARAMS})
