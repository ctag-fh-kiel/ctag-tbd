## Rgb indicator class ##

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

## properties ##

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


## methods ##

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
