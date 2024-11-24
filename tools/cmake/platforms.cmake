include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

set(TBD_PLATFORM_SYSTEMS esp32 desktop)
set(TBD_PLATFORM_CV_CHIPS adc mcp3208 stm32 midi)
set(TBD_PLATFORM_AUDIO_CHIPS wm8731 wm8978 wm8974 aic3254 es8388 rtaudio)
set(TBD_PLATFORM_INDICATORS no rgb neopixel)
set(TBD_PLATFORM_APIS wifi serial)


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
macro(tbd_platform_check)
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
        VOLUME_CONTROL 
        FILE_SYSTEM 
        DISPLAY
    )
    set(attrs
        NAME 
        SYSTEM 
        ARCH
        CV_INPUT  
        N_TRIGGERS 
        N_CVS
        AUDIO_OUTPUT 
        INDICATOR
    )
    set(multi_attrs
        APIS
    )
    cmake_parse_arguments(arg "${bools}" "${attrs}" "${multi_attrs}" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()

# @brief constructor for platform description
#
# @arg NAME [str]   name of the device
# @arg SYSTEM [enum]   platform/toolchain (desktop/esp32)
# @arg NAME [enum]   CV input type (adc/mcp3208/stm32/midi)
# @arg NAME [int]   number if triggers
# @arg NAME [int]   number of CV input channels
# @arg AUDIO_OUTPUT [enum]   audio chip name (wm8731/wm8978/wm8974/aic3254/es8388)
# @arg INDICATOR [enum]   indicator light type (no/rgb/neopixel)
# @arg VOLUME_CONTROL [flag]   do not release the SPI connection to the audio chip
# @arg FILE_SYSTEM [flag]   link file system driver
# @arg DISPLAY [flag]   link display driver
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

function(tbd_platform_arch platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_ARCH}" ${ARGN})
endfunction()

function(tbd_platform_cv_input platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_CV_INPUT}" ${ARGN})
endfunction()

function(tbd_platform_volume_control platform)
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
    tbd_store_or_return("${arg_AUDIO_OUTPUT}" ${ARGN})
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

# linearized summary of active features
#
# Creates a list of all active features 
#
#   [feature1=0, feature2=1 feature3=1, feature4=0, ...]
#
# to generate compiler defines etc.
#
function(tbd_platform_get_features platform)
    tbd_platform_attrs(${platform})
    if ("${arg_CV_INPUT}" STREQUAL "adc" OR "${arg_CV_INPUT}" STREQUAL "mcp3208")
        set(callibration TBD_CALIBRATION=1)
        set(adc TBD_CV_ADC=1)
        set(stm32 TBD_CV_STM32=0)
        set(midi TBD_CV_MIDI=0)
    elseif ("${arg_CV_INPUT}" STREQUAL "stm32")
        set(callibration TBD_CALIBRATION=0)
        set(adc TBD_CV_ADC=0)
        set(stm32 TBD_CV_STM32=1)
        set(midi TBD_CV_MIDI=0)
    elseif ("${arg_CV_INPUT}" STREQUAL "midi")
        set(callibration TBD_CALIBRATION=0)
        set(adc TBD_CV_ADC=0)
        set(stm32 TBD_CV_STM32=0)
        set(midi TBD_CV_MIDI=1)
    else()
        tbd_loge("failed to determine CV input, unknwon input chip ${arg_CV_INPUT}")
    endif()

    foreach (audio_chip IN LISTS TBD_PLATFORM_AUDIO_CHIPS)
        string(TOUPPER "${audio_chip}" audio_chip_upper)
        if ("${audio_chip}" STREQUAL "${arg_AUDIO_OUTPUT}")
            list(APPEND audio_chips "TBD_AUDIO_${audio_chip_upper}=1")
        else()
            list(APPEND audio_chips "TBD_AUDIO_${audio_chip_upper}=0")
        endif()
    endforeach()

    foreach (indicator IN LISTS TBD_PLATFORM_INDICATORS)
        string(TOUPPER "${indicator}" indicator_upper)
        if ("${indicator}" STREQUAL "${arg_INDICATOR}" AND NOT "${indicator}" STREQUAL "no")
            list(APPEND indicators "TBD_INDICATOR_${indicator_upper}=1")
        else()
            list(APPEND indicators "TBD_INDICATOR_${indicator_upper}=0")
        endif()
    endforeach()

    if (arg_VOLUME_CONTROL)
        set(volume_control TBD_VOLUME_CONTROL=1)
    else()
        set(volume_control TBD_VOLUME_CONTROL=0)
    endif()

    if (arg_FILE_SYSTEM)
        set(file_system TBD_FILE_SYSTEM=1)
    else()
        set(file_system TBD_FILE_SYSTEM=0)
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
        ${audio_chips}
        ${volume_control}
        ${indicators}
        ${file_system}
    )
    tbd_store_or_return("${features}" ${ARGN})
endfunction()

function (tbd_platform_print_features platform)
    tbd_log("${platform}")
    tbd_platform_get_features("${platform}")
    message("global feature flags")
    message("--------------------")
    foreach (feature IN LISTS _return)
        message("${feature}")
    endforeach()
    message("--------------------")
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
volume_control: ${arg_VOLUME_CONTROL}
indicators: ${arg_INDICATOR}
apis: ${arg_APIS}
file system: ${arg_FILE_SYSTEM}
display: ${arg_DISPLAY}
--------------------------
    ")
endfunction()


function(tbd_platform_load_preset file)
    file(READ "${file}" json_data)

    string(JSON config_version GET "${json_data}" config_version)
    if (NOT ${config_version} EQUAL 1)
        tbd_loge("platform config file version '${config_version}' is not compatible! expected 1")
    endif()

    # get data from `info`
    string(JSON info_obj GET "${json_data}" info)
    string(JSON name GET "${info_obj}" name)

    # get data from `config`
    string(JSON config_obj GET "${json_data}" config)
    string(JSON system GET "${config_obj}" system)
    string(JSON arch GET "${config_obj}" arch)
    string(JSON cv_inputs_obj GET "${config_obj}" cv_inputs)
    string(JSON audio_output_obj GET "${config_obj}" audio_output)
    string(JSON indicator GET "${config_obj}" indicator)
    string(JSON apis_obj GET "${config_obj}" apis)

    string(JSON file_system GET "${config_obj}" file_system)
    if ("${file_system}") 
        set(file_system "FILE_SYSTEM")
    else()
        unset(file_system)
    endif()

    string(JSON display GET "${config_obj}" display)
    if ("${display}") 
        set(display "DISPLAY")
    else()
        unset(display)
    endif()

    
    # get data from `config.cv_inputs`
    string(JSON cv_input GET "${cv_inputs_obj}" type)
    string(JSON n_cvs GET "${cv_inputs_obj}" n_cvs)
    string(JSON n_triggers GET "${cv_inputs_obj}" n_triggers)
    
    # get data form `config.audio_output`
    string(JSON audio_output GET "${audio_output_obj}" type)
    string(JSON volume_control GET "${audio_output_obj}" volume_control)
    if ("${volume_control}") 
        set(volume_control "VOLUME_CONTROL")
    else()
        unset(volume_control)
    endif()

    # extract data from `config.apis` array
    string(JSON num_apis LENGTH "${config_obj}" apis)
    foreach(i RANGE 1 "${num_apis}")
        math(EXPR i "${i} - 1")
        string(JSON api GET "${apis_obj}" ${i})
        tbd_log("API ${api}")
        list(APPEND apis "${api}")
    endforeach()

    tbd_platform(new_platform 
        NAME "${name}"
        SYSTEM "${system}"
        ARCH "${arch}"
        CV_INPUT "${cv_input}"
        N_CVS "${n_cvs}"
        N_TRIGGERS "${n_triggers}"
        AUDIO_OUTPUT ${audio_output}
        ${volume_control}
        INDICATOR ${indicator}
        APIS ${apis}
        ${file_system}
        ${display}
    )
    tbd_store_or_return("${new_platform}" ${ARGN})

endfunction()



### platform presets ####

function (tbd_platform_from_preset platform_name)
    set(config_file "${CMAKE_SOURCE_DIR}/config/platforms/platform.${platform_name}.json")
    tbd_platform_load_preset("${config_file}" VAR new_platform)
    tbd_store_or_return("${new_platform}" ${ARGN})
endfunction()
