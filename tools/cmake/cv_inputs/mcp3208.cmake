## Mcp3208 cv_input class ##

set(TBD_MCP3208_PINS cs mosi miso sclk)

# helper for accessing Mcp3208 fields
#
#
macro(tbd_mcp3208_attrs)
    set(attrs
            ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for mcp3208 cv_input
#
# @arg TYPE [enum]           has to be 'mcp3208'
# @arg PINS [map<str,int>]   spi connection pinout
#
function(tbd_mcp3208 var_name)
    tbd_mcp3208_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "mcp3208")
        tbd_loge("mcp3208 cv_input type has to be 'mcp3208' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_MCP3208_PINS})
    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()


## mcp3208 properties ##

function(tbd_mcp3208_pins mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_mcp3208_pin_flags mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_MCP3208_PINS}
            NAMESPACE TBD_MCP3208_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## Mcp3208 methods ##

function(tbd_mcp3208_print_info mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_MCP3208_PINS} VAR pins)
    message("
TBD cv_input configuration
---------------------------
type: mcp3208
${pins}---------------------------
    ")
endfunction()

function(_tbd_mcp3208_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)
    tbd_pinout_load("${pins_data}" PINS ${TBD_MCP3208_PINS} VAR pins)

    set(new_mcp3208
            PINS ${pins}
    )
    tbd_store_or_return("${new_mcp3208}" ${ARGN})
endfunction()
