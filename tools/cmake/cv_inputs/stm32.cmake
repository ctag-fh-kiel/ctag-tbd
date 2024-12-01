## Stm32 CV input ##

set(TBD_STM32_PINS cs mosi miso sclk)

# helper for accessing Stm32 fields
#
#
macro(tbd_stm32_attrs)
    set(attrs
            ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for stm32 cv_input
#
# @arg TYPE [enum]   has to be 'stm32'
#
function (tbd_stm32 var_name)
    tbd_stm32_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "stm32")
        tbd_loge("stm32 cv_input type has to be 'stm32' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_STM32_PINS})
    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## stm32 attrs ##

function(tbd_stm32_pins stm32)
    tbd_mcp3208_attrs(${stm32})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_stm32_pin_flags stm32)
    tbd_mcp3208_attrs(${stm32})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_STM32_PINS}
            NAMESPACE TBD_STM32_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## stm32 methods ##

function(tbd_stm32_print_info stm32)
    tbd_stm32_attrs(${stm32})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_STM32_PINS} VAR pins)
    message("
TBD cv_input configuration
---------------------------
type: stm32
${pins}---------------------------
    ")
endfunction()


function(_tbd_stm32_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)
    tbd_pinout_load("${pins_data}" PINS ${TBD_STM32_PINS} VAR pins)

    set(new_stm32
            PINS ${pins}
    )
    tbd_store_or_return("${new_stm32}" ${ARGN})
endfunction()
