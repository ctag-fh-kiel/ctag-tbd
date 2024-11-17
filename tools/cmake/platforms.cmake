include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

set(TBD_PLATFORM_SYSTEMS esp32 desktop)
set(TBD_PLATFORM_CV_CHIPS adc mcp3208 stm32 midi)
set(TBD_PLATFORM_AUDIO_CHIPS wm8xxx aic3254 es8388)
set(TBD_PLATFORM_INDICATORS rgb neopixel)


#### platform setup ####

# check if platform has been set up
#
# `TBD_PLATFORM` has to refer to a valid platform preset platform.
# 
# @note: Use `tbd_platform_setup` make the selected platform globally available.
#
macro(tbd_platform_is_set)
    if ("${TBD_PLATFORM}" STREQUAL "")
        tbd_loge("TBD_PLATFORM not set, add '-DTBD_PLATFORM=<platform>' to idf.py or cmake")
    endif()
endmacro()


# determine which platform to build
#
# This should be the first thing to do in your main `CMakeLists.txt`.
#
# There is two options for specifying the platform to build (ordered by priority
# 1. directly on the command line:
#
#   If you not using the TBD tools, set 
#     `cmake -DTBD_PLATFORM=<platform>`
#   or
#      `idf.py -DTBD_PLATFORM`
#   on the build tool you are using.
# 2. as an environment variable:
#
#   export TBD_PLATFORM=<platform>
#
#
macro(tbd_platform_setup)
    tbd_logv("determining platform")

    if (NOT "${TBD_PLATFORM}" STREQUAL "")
        tbd_logv("using platform ${TBD_PLATFORM} from cmake variable")
        set(platform_name ${TBD_PLATFORM})
    elseif (NOT "$ENV{TBD_PLATFORM}" STREQUAL "")
        tbd_logv("using platform ${TBD_PLATFORM} set from environment variable")
        set(platform_name $ENV{TBD_PLATFORM})
    else()
        tbd_loge("TBD_PLATFORM not set, add '-DTBD_PLATFORM=<platform>' to idf.py or cmake")
    endif()

    set(TBD_PLATFORM ${platform_name} CACHE STRING "" FORCE)
    mark_as_advanced(TBD_PLATFORM)
endmacro()


# activate platfom
#
# Make the platfom available to the entire project as `TBD_PLATFORM_OBJ`.
#
macro(tbd_platform_activate)
    tbd_logv("activating platform")

    if ("${TBD_PLATFORM}" STREQUAL "")
        tbd_loge("TBD_PLATFORM not set: add '-DTBD_PLATFORM=<platform>' to idf.py or cmake or set TBD_PLATFORM envvar")
    endif()

    tbd_platform_is_set()
    tbd_platform_from_preset(${TBD_PLATFORM})
    set(TBD_PLATFORM_OBJ "${_return}" CACHE STRING "" FORCE)
    mark_as_advanced(TBD_PLATFORM_OBJ)

    tbd_platform_print_info("${TBD_PLATFORM_OBJ}")
endmacro()


# raise an error if plotform has not been activated
#
#
macro(tbd_platform_activated)
    if ("${TBD_PLATFORM_OBJ}" STREQUAL "")
        tbd_loge("TBD_PLATFORM_OBJ not set, did you forget to call 'tbd_platform_actiate'?")
    endif()
endmacro()


#### platform class ####

# helper for accessing platform fields
#
#
macro(tbd_platform_attrs)
    set(bools 
        FILE_SYSTEM 
        DISPLAY
    )
    set(attrs
        NAME 
        SYSTEM 
        CV_INPUT  
        N_TRIGGERS 
        N_CVS
        AUDIO_OUTPUT 
        INDICATOR
    )
    cmake_parse_arguments(arg "${bools}" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()

#
#  constructor for platform description
#
function (tbd_platform var_name)
    tbd_platform_attrs(${ARGN})

    # check if speficied components are valid
    list (FIND TBD_PLATFORM_SYSTEMS ${arg_SYSTEM} _index)
    if (${_index} LESS 0) 
        tbd_loge("unknwon system '${arg_SYSTEM}, has to be one of ${TBD_PLATFTBD_PLATFORM_SYSTEMS}")
    endif()

    list (FIND TBD_PLATFORM_CV_CHIPS ${arg_CV_INPUT} _index)
    if (${_index} LESS 0) 
        tbd_loge("unknwon CV chip '${arg_CV_INPUT}, has to be one of ${TBD_PLATFORM_CV_CHIPS}")
    endif()

    list (FIND TBD_PLATFORM_AUDIO_CHIPS ${arg_AUDIO_OUTPUT} _index)
    if (${_index} LESS 0) 
        tbd_loge("unknwon audio chip '${arg_AUDIO_OUTPUT}, has to be one of ${TBD_PLATFORM_AUDIO_CHIPS}")
    endif()

    list (FIND TBD_PLATFORM_INDICATORS ${arg_INDICATOR} _index)
    if (${_index} LESS 0) 
        tbd_loge("unknwon indicator '${arg_INDICATOR}, has to be one of ${TBD_PLATFORM_INDICATORS}")
    endif()

    set(${var_name} ${ARGN} PARENT_SCOPE)
endfunction()

# platform properties

function(tbd_platform_name platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_NAME}" ${ARGN})
endfunction()

function(tbd_platform_system platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_SYSTEM}" ${ARGN})
endfunction()

function(tbd_platform_cv_input platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_CV_INPUT}" ${ARGN})
endfunction()

function(tbd_platform_n_cvs platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_N_CVS}" ${ARGN})
endfunction()

function(tbd_platform_n_triggers platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_N_TRIGGERS}" ${ARGN})
endfunction()

function(tbd_platform_audio_output platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_AUDIO_OUTPUT" ${ARGN})
endfunction()

function(tbd_platform_file_system platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_FILE_SYSTEM}" ${ARGN})
endfunction()

function(tbd_platform_display platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_DIPLAY}" ${ARGN})
endfunction()

function(tbd_platform_indicators platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_INDICATORS}" ${ARGN})
endfunction()

#### platform methods ####

function(tbd_platform_get_features platform)
    tbd_platform_attrs(${platform})
    if ("${arg_CV_INPUT}" STREQUAL "adc" OR "${arg_CV_INPUT}" STREQUAL "mcp3208")
        set(callibration TBD_CALIBRATION=1)
        set(adc TBD_ADC=1)
        set(stm32 TBD_STM32=0)
        set(midi TBD_MIDI=0)
    elseif ("${arg_CV_INPUT}" STREQUAL "stm32")
        set(callibration TBD_CALIBRATION=0)
        set(adc TBD_ADC=0)
        set(stm32 TBD_STM32=1)
        set(midi TBD_MIDI=0)
    elseif ("${arg_CV_INPUT}" STREQUAL "midi")
        set(callibration TBD_CALIBRATION=0)
        set(adc TBD_ADC=0)
        set(stm32 TBD_STM32=0)
        set(midi TBD_MIDI=1)
    else()
        tbd_loge("failed to determine CV input, unknwon input chip ${arg_CV_INPUT}")
    endif()

    if (arg_DISPLAY)
        set(display TBD_DISPLAY=1)
    else()
        set(display TBD_DISPLAY=0)
    endif()

    set(features
        N_CVS=${arg_N_CVS}
        N_TRIGS=${arg_N_TRIGGERS}
        ${callibration}
        ${adc}
        ${stm32}
        ${midi}
        ${display}
    )
    tbd_store_or_return("${features}" ${ARGN})
endfunction()

function(tbd_platform_print_info platform)
    tbd_platform_attrs(${platform})
    message("
TBD platform configuration
--------------------------
name: ${arg_NAME}
system: ${arg_SYSTEM}
CV chip: ${arg_CV_INPUT}
num CVs: ${arg_N_CVS}
num triggers: ${arg_N_TRIGGERS}
audio chip: ${arg_AUDIO_OUTPUT}
file system: ${arg_FILE_SYSTEM}
indicators: ${arg_INDICATOR}
--------------------------
    ")
endfunction()


### platform presets ####

function (tbd_platform_from_preset platform_name)
    if (${platform_name} STREQUAL "v1")
        tbd_platform(new_platform 
            NAME "v1"
            SYSTEM esp32
            CV_INPUT adc 
            N_CVS 4
            N_TRIGGERS 2
            AUDIO_OUTPUT wm8xxx 
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR rgb 
        )

    elseif (${platform_name} STREQUAL "v2")
        tbd_platform(new_platform 
            NAME "v2"
            SYSTEM esp32
            CV_INPUT adc 
            N_CVS 4
            N_TRIGGERS 2
            AUDIO_OUTPUT wm8xxx 
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR rgb 
        )

    elseif (${platform_name} STREQUAL "str")
        tbd_platform(new_platform 
            NAME "str"
            SYSTEM esp32
            CV_INPUT mcp3208 
            N_CVS 8
            N_TRIGGERS 2
            AUDIO_OUTPUT wm8xxx 
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR rgb 
        )

    elseif (${platform_name} STREQUAL "aem")
        tbd_platform(new_platform 
            NAME "aem"
            SYSTEM esp32
            CV_INPUT adc 
            N_CVS 4
            N_TRIGGERS 2
            AUDIO_OUTPUT wm8xxx 
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR rgb 
        )

    elseif (${platform_name} STREQUAL "mk2")
        tbd_platform(new_platform 
            NAME "mk2"
            SYSTEM esp32
            CV_INPUT stm32 
            N_CVS 22
            N_TRIGGERS 12
            AUDIO_OUTPUT wm8xxx 
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR rgb 
        )

    elseif (${platform_name} STREQUAL "bba1")
        tbd_platform(new_platform 
            NAME "bba1"
            SYSTEM esp32
            CV_INPUT midi 
            N_CVS 90
            N_TRIGGERS 40
            AUDIO_OUTPUT es8388
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR neopixel
        )

    elseif (${platform_name} STREQUAL "bba2")
        tbd_platform(new_platform 
            NAME "bba2" 
            SYSTEM esp32 
            CV_INPUT midi 
            N_CVS 90 
            N_TRIGGERS 40 
            AUDIO_OUTPUT es8388
            FILE_SYSTEM 
            DISPLAY 
            INDICATOR neopixel
        )
    endif()

    tbd_store_or_return("${new_platform}" ${ARGN})
endfunction()
