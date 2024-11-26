include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

set(TBD_INDICATOR_TYPES no rgb neopixel)


#### RBG indicator class ####


# helper for accessing RGB fields
#
#
macro(tbd_rgb_attrs)
    set(attrs
        TYPE
        RED
        GREEN
        BLUE
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for rgb indicators
#
# @arg TYPE [enum]   has to be 'rgb'
# @arg RED [int]     pin of red led
# @arg RED [int]     pin of green led
# @arg RED [int]     pin of blue led
#
function (tbd_rgb var_name)
    tbd_rgb_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "rgb")
        tbd_loge("rgb indicator type has to be 'rgb' got '${arg_TYPE}'")
    endif()

    tbd_check_int("${arg_RED}")
    tbd_check_int("${arg_GREEN}")
    tbd_check_int("${arg_BLUE}")

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## rgb properties ##

function(tbd_rgb_red rgb)
    tbd_rgb_attrs(${rgb})
    tbd_store_or_return("${arg_RED}" ${ARGN})
endfunction()

function(tbd_rgb_green rgb)
    tbd_rgb_attrs(${rgb})
    tbd_store_or_return("${arg_GREEN}" ${ARGN})
endfunction()

function(tbd_rgb_blue rgb)
    tbd_rgb_attrs(${rgb})
    tbd_store_or_return("${arg_BLUE}" ${ARGN})
endfunction()

## rbg methods ##

function(tbd_rgb_print_info rgb)
    tbd_rgb_attrs(${rgb})
    message("
TBD indicator configuration
---------------------------
type: rgb
red pin:   ${arg_RED}
green pin: ${arg_GREEN}
blue pin:  ${arg_BLUE}
---------------------------
    ")
endfunction()

function(tbd_rgb_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)

    string(JSON red GET "${pins_data}" red)
    string(JSON green GET "${pins_data}" green)
    string(JSON blue GET "${pins_data}" blue)

    tbd_rgb(new_rgb 
        TYPE ${type}
        RED ${red}    
        GREEN ${green}
        BLUE ${blue}
    )
    tbd_store_or_return("${new_rgb}" ${ARGN})
endfunction()


#### neopixel indicator class ####

# helper for accessing Neopixel fields
#
#
macro(tbd_neopixel_attrs)
    set(attrs
        TYPE
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
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

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## neopixel methods ##

function(tbd_neopixel_print_info neopixel)
    message("
TBD indicator configuration
---------------------------
type: neopixel
---------------------------
    ")
endfunction()


function(tbd_neopixel_load json_data)
    string(JSON type GET "${json_data}" type)
    tbd_neopixel(new_neopixel 
        TYPE ${type}
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