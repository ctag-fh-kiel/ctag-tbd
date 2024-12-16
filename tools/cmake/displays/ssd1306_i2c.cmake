## ssd1306 display class ##

set(TBD_SSD1306_I2C_PINS sda scl reset)

# helper for accessing ssd1306_i2c fields
#
#
macro(tbd_ssd1306_i2c_attrs)
    tbd_display_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "ssd1306_i2c")
        tbd_loge("ssd1306_i2c display type has to be 'ssd1306_i2c' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_SSD1306_I2C_PINS})
endmacro()


# @brief constructor for ssd1306_i2c displays
#
# @arg TYPE [enum]     has to be 'ssd1306_i2c'
# @arg PINS [struct]   i2c and rest connection pins
#
function (tbd_ssd1306_i2c var_name)
    tbd_ssd1306_i2c_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## ssd1306_i2c attrs ##

function(tbd_ssd1306_i2c_pins ssd1306_i2c)
    tbd_ssd1306_i2c_attrs(${ssd1306_i2c})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_ssd1306_i2c_pin_flags ssd1306_i2c)
    tbd_ssd1306_i2c_attrs(${ssd1306_i2c})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_SSD1306_I2C_PINS}
            NAMESPACE TBD_SSD1306_I2C_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## ssd1306_i2c methods ##

function(tbd_ssd1306_i2c_print_info ssd1306_i2c)
    tbd_ssd1306_i2c_attrs(${ssd1306_i2c})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_SSD1306_I2C_PINS} VAR pins)
    message("
TBD display configuration
---------------------------
type: ssd1306_i2c
${pins}---------------------------
    ")
endfunction()


function(_tbd_ssd1306_i2c_load json_data)
    string(JSON pins_data GET "${json_data}" pins)
    message("${json_data}")
    tbd_pinout_load("${pins_data}"
            PINS ${TBD_SSD1306_I2C_PINS}
            VAR pins
    )
    set(new_ssd1306_i2c
            PINS ${pins}
    )
    tbd_store_or_return("${new_ssd1306_i2c}" ${ARGN})
endfunction()
