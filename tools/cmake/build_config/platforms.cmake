include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/indicators.cmake)
include(${TBD_CMAKE_DIR}/cv_inputs.cmake)
include(${TBD_CMAKE_DIR}/codecs.cmake)
include(${TBD_CMAKE_DIR}/displays.cmake)

set(TBD_PLATFORM_SYSTEMS esp32 desktop)
set(TBD_PLATFORM_APIS wifi serial shell)
set(TBD_PLATFORM_FILE_SYSTEMS std wrapper)


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

    if ("${TBD_PLATFORM_FILE}" STREQUAL "")
        tbd_loge("TBD_PLATFORM_FILE not set, tbd_platform_setup must be called")
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

    set(TBD_PLATFORM_FILE "${CMAKE_SOURCE_DIR}/config/platforms/platform.${platform_name}.json" CACHE STRING "" FORCE)
    mark_as_advanced(TBD_PLATFORM_FILE)

    if (NOT EXISTS "${TBD_PLATFORM_FILE}")
        tbd_loge("no such platform: platform configuration file '${TBD_PLATFORM_FILE}' does not exist")
    endif()
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
    set(attrs
        NAME 
        SYSTEM 
        ARCH
        FILE_SYSTEM
        NETWORK
    )
    set(multi_attrs
        INPUTS
        CV_INPUT
        INDICATOR
        AUDIO
        APIS
        DISPLAY
    )
    cmake_parse_arguments(arg "${bools}" "${attrs}" "${multi_attrs}" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for platform description
#
# @arg NAME [str]              name of the device
# @arg SYSTEM [enum]           platform/toolchain (desktop/esp32)
# @arg ARCH [enum]             exact CPU/SoC (desktop/esp32/esp32s3)
# @arg CV_INPUT [class]        CV input config
# @arg NAME [int]              number if triggers
# @arg NAME [int]              number of CV input channels
# @arg AUDIO [class]           audio chip config
# @arg INDICATOR [class]       indicator light type
# @arg FILE_SYSTEM [enum]      pick between stdlib filesystem and custom subset (std/wrapper)
# @arg DISPLAY [class]         configure connected display
# @arg NETWORK [bool]          enable WIFI and WIFI control
#
function (tbd_platform var_name)
    tbd_platform_attrs(${ARGN})

    # check if speficied components are valid
    list (FIND TBD_PLATFORM_SYSTEMS ${arg_SYSTEM} _index)
    if (${_index} LESS 0) 
        tbd_loge("unknown system '${arg_SYSTEM}' has to be one of ${TBD_PLATFTBD_PLATFORM_SYSTEMS}")
    endif()

    if (NOT "${arg_FILE_SYSTEM}" IN_LIST TBD_PLATFORM_FILE_SYSTEMS)
        tbd_loge("unknown file system '${arg_FILE_SYSTEM}' has to be one of ${TBD_PLATFORM_FILE_SYSTEMS}")
    endif()

    tbd_indicator(CHECK "${arg_INDICATOR}")

    if(NOT "${arg_DISPLAY}" STREQUAL no)
        tbd_display(CHECK "${arg_DISPLAY}")
    endif()
    tbd_cv_input(CHECK "${arg_CV_INPUT}")
    tbd_codec(CHECK "${arg_AUDIO}")
    tbd_check_bool("${arg_NETWORK}")

    if (NOT "${var_name}" STREQUAL "_")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## platform properties ##

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

function(tbd_platform_n_cvs platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_N_CVS}" ${ARGN})
endfunction()

function(tbd_platform_n_triggers platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_N_TRIGGERS}" ${ARGN})
endfunction()

function(tbd_platform_audio platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_AUDIO}" ${ARGN})
endfunction()

function(tbd_platform_file_system platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_FILE_SYSTEM}" ${ARGN})
endfunction()

function(tbd_platform_display platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_DISPLAY}" ${ARGN})
endfunction()

function(tbd_platform_indicator platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_INDICATOR}" ${ARGN})
endfunction()

function(tbd_platform_apis platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_APIS}" ${ARGN})
endfunction()

function(tbd_platform_network platform)
    tbd_platform_attrs(${platform})
    tbd_store_or_return("${arg_NETWORK}" ${ARGN})
endfunction()

## platform methods ##

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

    tbd_cv_input_type_flags("${arg_CV_INPUT}" VAR cv_flags)

    # FIXME: this should be either be done in the input library or all inputs should
    #        allow calibration
    tbd_cv_input_needs_calibration("${arg_CV_INPUT}")
    if (_return)
        set(calibration TBD_CALIBRATION=1)
    else()
        set(calibration TBD_CALIBRATION=0)
    endif()

    tbd_cv_input_n_cvs("${arg_CV_INPUT}" VAR n_cvs)
    tbd_cv_input_n_triggers("${arg_CV_INPUT}" VAR n_triggers)

    if("${arg_DISPLAY}" STREQUAL no)
        set(display TBD_DISPLAY=0)
    else()
        set(display TBD_DISPLAY=1)
    endif()

    set(features
        N_CVS=${n_cvs}
        N_TRIGS=${n_triggers}
        ${calibration}
        ${cv_flags}
        ${display}
        ${volume_control}
        INDICATOR=${indicators}
        ${audio_mechanism}
    )

    tbd_store_or_return("${features}" ${ARGN})
endfunction()

function (tbd_platform_print_features platform)
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
    tbd_indicator_type("${arg_INDICATOR}" VAR indicator_type)
    tbd_cv_input_type("${arg_CV_INPUT}" VAR cv_type)
    tbd_cv_input_n_cvs("${arg_CV_INPUT}" VAR n_cvs)
    tbd_cv_input_n_triggers("${arg_CV_INPUT}" VAR n_triggers)
    tbd_codec_type("${arg_AUDIO}" VAR codec_type)
    if(NOT "${arg_DISPLAY}" STREQUAL no)
        tbd_display_type("${arg_DISPLAY}" VAR display_type)
    else()
        set(display_type -)
    endif()

    message("
TBD platform configuration
--------------------------
name: ${arg_NAME}
system: ${arg_SYSTEM}
CV chip: ${cv_type}
num CVs: ${n_cvs}
num triggers: ${n_triggers}
audio chip: ${codec_type}
indicators: ${indicator_type}
apis: ${arg_APIS}
file system: ${arg_FILE_SYSTEM}
display: ${display_type}
netwrok: ${arg_NETWORK}
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
    string(JSON apis_obj GET "${config_obj}" apis)
    string(JSON file_system GET "${config_obj}" file_system)

    string(JSON display_type TYPE "${config_obj}" display)
    if("${display_type}" STREQUAL NULL)
        set(display no)
    else()
        string(JSON display_obj GET "${config_obj}" display)
        tbd_display_load("${display_obj}" VAR display)
    endif()

    string(JSON network GET "${config_obj}" network)
    tbd_to_bool(network)

    string(JSON indicator_obj GET "${config_obj}" indicator)
    tbd_indicator_load("${indicator_obj}" VAR indicator)

    string(JSON cv_input_obj GET "${config_obj}" cv_input)
    tbd_cv_input_load("${cv_input_obj}" VAR cv_input)

    string(JSON audio_obj GET "${config_obj}" audio)
    tbd_codec_load("${audio_obj}" VAR audio)

    # extract data from `config.apis` array
    string(JSON num_apis LENGTH "${config_obj}" apis)
    foreach(i RANGE 1 "${num_apis}")
        math(EXPR i "${i} - 1")
        string(JSON api GET "${apis_obj}" ${i})
        list(APPEND apis "${api}")
    endforeach()

    tbd_platform(new_platform 
        NAME "${name}"
        SYSTEM "${system}"
        ARCH "${arch}"
        CV_INPUT "${cv_input}"
        AUDIO "${audio}"
        INDICATOR ${indicator}
        APIS ${apis}
        FILE_SYSTEM ${file_system}
        DISPLAY "${display}"
        NETWORK ${network}
    )
    tbd_store_or_return("${new_platform}" ${ARGN})
endfunction()


#### platform presets ####

function (tbd_platform_from_preset platform_name)
    set(config_file "${CMAKE_SOURCE_DIR}/config/platforms/platform.${platform_name}.json")
    tbd_platform_load_preset("${config_file}" VAR new_platform)
    tbd_store_or_return("${new_platform}" ${ARGN})
endfunction()
