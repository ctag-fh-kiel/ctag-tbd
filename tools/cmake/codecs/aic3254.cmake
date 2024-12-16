## aic3254 codec class ##

set(TBD_AIC3254_I2C_PINS sda scl)
set(TBD_AIC3254_I2S_PINS mclk bclk ws dout din)

# helper for accessing aic3254 fields
#
#
macro(tbd_aic3254_attrs)
    tbd_codec_attrs(${ARGV})
    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "I2C;I2S" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "aic3254")
        tbd_loge("aic3254 codec type has to be 'aic3254' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_I2C}" PINS ${TBD_AIC3254_I2C_PINS})
    tbd_pinout_check("${arg_I2S}" PINS ${TBD_AIC3254_I2S_PINS})
endmacro()


# @brief constructor for aic3254
#
# @arg TYPE [enum]           has to be 'aic3254'
# @arg SPI [map<str,int>]    control communication pins
# @arg I2S [map<str,int>]    audio communication pins
#
function (tbd_aic3254 var_name)
    tbd_aic3254_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

# function(TBD_AIC3254_PINS aic3254)
#     tbd_aic3254_attrs(${aic3254})
#     tbd_store_or_return("${arg_PINS}" ${ARGN})
# endfunction()

function(tbd_aic3254_i2c_pins aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_store_or_return("${arg_SPI}" ${ARGN})
endfunction()

function(tbd_aic3254_i2s_pins aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_store_or_return("${arg_I2S}" ${ARGN})
endfunction()

function(tbd_aic3254_pin_flags aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_pinout_flags("${arg_I2C}"
            PINS ${TBD_AIC3254_I2C_PINS}
            NAMESPACE TBD_AIC3254_I2C_PIN_
            PREFIX GPIO_NUM_
            VAR i2c
    )
    tbd_pinout_flags("${arg_I2S}"
            PINS ${TBD_AIC3254_I2S_PINS}
            NAMESPACE TBD_AIC3254_I2S_PIN_
            PREFIX GPIO_NUM_
            VAR i2s
    )
    tbd_store_or_return("${i2c};${i2s}" ${ARGN})
endfunction()

## methods ##

function(tbd_aic3254_print_info aic3254)
    tbd_aic3254_attrs(${aic3254})
    tbd_pinout_info("${arg_I2C}"
            PINS ${TBD_AIC3254_I2C_PINS}
            VAR i2c
    )
    tbd_pinout_info("${arg_I2S}"
            PINS ${TBD_AIC3254_I2S_PINS}
            VAR i2s
    )
    message("
TBD codec configuration
---------------------------
type: aic3254

i2c pins
........
${i2c}
i2c pins
........
${i2s}---------------------------
    ")
endfunction()

function(_tbd_aic3254_load json_data)
    string(JSON pin_obj GET "${json_data}" pins)

    string(JSON i2c_data GET "${pin_obj}" i2c)
    tbd_pinout_load("${i2c_data}"
            PINS ${TBD_AIC3254_I2C_PINS}
            VAR i2c
    )

    string(JSON i2s_data GET "${pin_obj}" i2s)
    tbd_pinout_load("${i2s_data}"
            PINS ${TBD_AIC3254_I2S_PINS}
            VAR i2s
    )

    set(new_aic3254
        I2C "${i2c}"
        I2S "${i2s}"
        WORK_TYPE pull
    )
    tbd_store_or_return("${new_aic3254}" ${ARGN})
endfunction()
