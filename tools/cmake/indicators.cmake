include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

set(TBD_INDICATOR_TYPES no rgb neopixel)


#### RBG indicator class ####

set(TBD_RGB_PINS red green blue)

# helper for accessing RGB fields
#
#
macro(tbd_rgb_attrs)
    cmake_parse_arguments(arg "" "TYPE" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for rgb indicators
#
# @arg TYPE [enum]           has to be 'rgb'
# @arg PINS [map<str,int>]   led pins
#
function (tbd_rgb var_name)
    tbd_rgb_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "rgb")
        tbd_loge("rgb indicator type has to be 'rgb' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_RGB_PINS})
    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## rgb properties ##

function(tbd_rgb_pins rgb)
    tbd_rgb_attrs(${rgb})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_rgb_pin_flags rgb)
    tbd_rgb_attrs(${rgb})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_RGB_PINS}
            NAMESPACE TBD_RGB_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()


## rbg methods ##

function(tbd_rgb_print_info rgb)
    tbd_rgb_attrs(${rgb})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_RGB_PINS} VAR pins)

    message("
TBD indicator configuration
---------------------------
type: rgb
${pins}---------------------------
    ")
endfunction()

function(tbd_rgb_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)
    tbd_pinout_load("${pins_data}"
            PINS ${TBD_RGB_PINS}
            VAR pins
    )

    tbd_rgb(new_rgb 
        TYPE ${type}
        PINS ${pins}
    )
    tbd_store_or_return("${new_rgb}" ${ARGN})
endfunction()


#### neopixel indicator class ####

set(TBD_NEOPIXEL_PINS dout)

# helper for accessing Neopixel fields
#
#
macro(tbd_neopixel_attrs)
    cmake_parse_arguments(arg "" "TYPE" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for neopixel indicators
#
# @arg TYPE [enum]   has to be 'neopixel'
#
function (tbd_neopixel var_name)
    tbd_neopixel_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "neopixel")
        tbd_loge("neopixel indicator type has to be 'neopixel' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_NEOPIXEL_PINS})
    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## Neopixel attrs ##

function(tbd_neopixel_pins neopixel)
    tbd_neopixel_attrs(${neopixel})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_neopixel_pin_flags neopixel)
    tbd_neopixel_attrs(${neopixel})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_NEOPIXEL_PINS}
            NAMESPACE TBD_NEOPIXEL_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## neopixel methods ##

function(tbd_neopixel_print_info neopixel)
    tbd_neopixel_attrs(${neopixel})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_NEOPIXEL_PINS} VAR pins)
    message("
TBD indicator configuration
---------------------------
type: neopixel
${pins}---------------------------
    ")
endfunction()


function(tbd_neopixel_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)
    tbd_pinout_load("${pins_data}"
            PINS ${TBD_NEOPIXEL_PINS}
            VAR pins
    )
    tbd_neopixel(new_neopixel 
        TYPE ${type}
        PINS ${pins}
    )
    tbd_store_or_return("${new_neopixel}" ${ARGN})
endfunction()


#### indicator base class ####

# helper for accessing indicator fields
#
#
macro(tbd_indicator_attrs)
    set(attrs
        TYPE
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for all indicator types
#
# @arg TYPE [enum]   type of indicator
#
function (tbd_indicator var_name)
    tbd_indicator_attrs(${ARGN})

    if ("${arg_TYPE}" STREQUAL "rgb")
        tbd_rgb(CHECK ${ARGN})
    elseif("${arg_TYPE}" STREQUAL "neopixel")
        tbd_neopixel(CHECK ${ARGN})
    else()
        tbd_loge("unknown indicator type ${arg_TYPE}")
    endif()
    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## indicator properties ##

function(tbd_indicator_type indicator)
    tbd_indicator_attrs(${indicator})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

## indicator methods ##

function(tbd_indicator_print_info indicator)
    tbd_indicator_attrs(${indicator})
    if ("${arg_TYPE}" STREQUAL "rgb")
        tbd_rgb_print_info("${indicator}")
    elseif("${arg_TYPE}" STREQUAL "neopixel")
        tbd_neopixel_print_info("${indicator}")
    else()
        message("invalid") 
    endif()
endfunction()

function(tbd_indicator_load json_data)
    string(JSON type GET "${json_data}" type)
    if ("${type}" STREQUAL "rgb")
        tbd_rgb_load("${json_data}" VAR new_indicator)
    elseif("${type}" STREQUAL "neopixel")
        tbd_neopixel_load("${json_data}" VAR new_indicator)
    else()
        tbd_loge("unknown indicator type '${type}' in json")
    endif()
    tbd_store_or_return("${new_indicator}" ${ARGN})
endfunction()