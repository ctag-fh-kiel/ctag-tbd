## aic3254 codec class ##

set(TBD_AIC3254_PINS mclk bclk ws dout din)

# helper for accessing aic3254 fields
#
#
macro(tbd_aic3254_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for aic3254
#
# @arg TYPE [enum]           has to be 'aic3254'
# @arg PINS [map<str,int>]   communication pins
#
function (tbd_aic3254 var_name)
    tbd_codec_attrs(${ARGN})
    tbd_aic3254_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "aic3254")
        tbd_loge("aic3254 codec type has to be 'aic3254' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_AIC3254_PINS})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

function(TBD_AIC3254_PINS aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_aic3254_pin_flags aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_AIC3254_PINS}
            NAMESPACE TBD_AIC3254_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()


## methods ##

function(tbd_aic3254_print_info aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_pinout_info("${arg_PINS}"
            PINS ${TBD_AIC3254_PINS}
            VAR pins
    )
    message("
TBD codec configuration
---------------------------
type: aic3254
${pins}---------------------------
    ")
endfunction()

function(_tbd_aic3254_load json_data)
    string(JSON type GET "${json_data}" type)

    string(JSON pins_obj GET "${json_data}" pins)
    tbd_pinout_load("${pins_obj}"
            PINS ${TBD_AIC3254_PINS}
            VAR pins
    )

    set(new_aic3254
        PINS "${pins}"
        WORK_TYPE pull
    )
    tbd_store_or_return("${new_aic3254}" ${ARGN})
endfunction()
