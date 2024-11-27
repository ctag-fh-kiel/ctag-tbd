## neopixel indicator class ##

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
