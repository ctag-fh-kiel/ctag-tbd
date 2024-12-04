## es8388 codec class ##

set(TBD_ES8388_PINS sda scl)

# helper for accessing es8388 fields
#
#
macro(tbd_es8388_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS};I2C_BUS" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for es8388
#
# @arg TYPE [enum]           has to be 'es8388'
# @arg PINS [int]            i2c bus unit to be used
# @arg PINS [map<str,int>]   i2c connection pins
#
function (tbd_es8388 var_name)
    tbd_es8388_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "es8388")
        tbd_loge("es8388 codec type has to be 'es8388' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_ES8388_PINS})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

function(tbd_es8388_pins es8388)
    tbd_es8388_attrs(${es8388})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_es8388_pin_flags es8388)
    tbd_es8388_attrs(${es8388})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_ES8388_PINS}
            NAMESPACE TBD_ES8388_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()


## methods ##

function(tbd_es8388_print_info es8388)
    tbd_es8388_attrs(${es8388})
    tbd_pinout_info("${arg_PINS}"
            PINS ${TBD_ES8388_PINS}
            VAR pins
    )
    message("
TBD codec configuration
---------------------------
type: es8388
${pins}---------------------------
    ")
endfunction()

function(_tbd_es8388_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON bus GET "${json_data}" i2c_bus)

    string(JSON pins_obj GET "${json_data}" pins)
    tbd_pinout_load("${pins_obj}"
            PINS ${TBD_ES8388_PINS}
            VAR pins
    )

    set(new_es8388
            TYPE ${type}
            I2C_BUS "${bus}"
            PINS "${pins}"
            WORK_TYPE pull
    )
    tbd_store_or_return("${new_es8388}" ${ARGN})
endfunction()
