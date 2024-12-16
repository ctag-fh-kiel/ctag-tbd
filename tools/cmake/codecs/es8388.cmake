## es8388 codec class ##

set(TBD_ES8388_I2C_PINS sda scl)
set(TBD_ES8388_I2S_PINS mclk bclk ws dout din)

# helper for accessing es8388 fields
#
#
macro(tbd_es8388_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "I2C;I2S" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "es8388")
        tbd_loge("es8388 codec type has to be 'es8388' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_I2C}" PINS ${TBD_ES8388_I2C_PINS})
    tbd_pinout_check("${arg_I2S}" PINS ${TBD_ES8388_I2S_PINS})
endmacro()


# @brief constructor for es8388
#
# @arg TYPE [enum]           has to be 'es8388'
# @arg I2C [map<str,int>]    control communication pins
# @arg I2S [map<str,int>]    audio communication pins
#
function (tbd_es8388 var_name)
    tbd_es8388_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

function(tbd_es8388_i2c_pins es8388)
    tbd_es8388_attrs(${es8388})
    tbd_store_or_return("${arg_I2C}" ${ARGN})
endfunction()

function(tbd_es8388_i2s_pins es8388)
    tbd_es8388_attrs(${es8388})
    tbd_store_or_return("${arg_I2S}" ${ARGN})
endfunction()

function(tbd_es8388_pin_flags es8388)
    tbd_es8388_attrs(${es8388})
    tbd_pinout_flags("${arg_I2C}"
            PINS ${TBD_ES8388_I2C_PINS}
            NAMESPACE TBD_ES8388_I2C_PIN_
            PREFIX GPIO_NUM_
            VAR i2c
    )
    tbd_pinout_flags("${arg_I2S}"
            PINS ${TBD_ES8388_I2S_PINS}
            NAMESPACE TBD_ES8388_I2S_PIN_
            PREFIX GPIO_NUM_
            VAR i2s
    )
    tbd_store_or_return("${i2c};${i2s}" ${ARGN})
endfunction()


## methods ##

function(tbd_es8388_print_info es8388)
    tbd_es8388_attrs(${es8388})
    tbd_pinout_info("${arg_I2C}"
            PINS ${TBD_ES8388_I2C_PINS}
            VAR i2c
    )
    tbd_pinout_info("${arg_I2S}"
            PINS ${TBD_ES8388_I2S_PINS}
            VAR i2s
    )
    message("
TBD codec configuration
---------------------------
type: es8388

i2c pins
........
${i2c}
i2c pins
........
${i2s}---------------------------
    ")
endfunction()

function(_tbd_es8388_load json_data)
    string(JSON pin_obj GET "${json_data}" pins)

    string(JSON i2c_data GET "${pin_obj}" i2c)
    tbd_pinout_load("${i2c_data}"
            PINS ${TBD_ES8388_I2C_PINS}
            VAR i2c
    )

    string(JSON i2s_data GET "${pin_obj}" i2s)
    tbd_pinout_load("${i2s_data}"
            PINS ${TBD_ES8388_I2S_PINS}
            VAR i2s
    )

    set(new_es8388
        I2C "${i2c}"
        I2S "${i2s}"
        WORK_TYPE pull
    )
    tbd_store_or_return("${new_es8388}" ${ARGN})
endfunction()

