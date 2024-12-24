## SPI API class ##

set(TBD_API_SPI_PINS clk mosi miso)

# helper for accessing API_SPI fields
#
#
macro(tbd_api_spi_attrs)
    tbd_api_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "PINS" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "spi")
        tbd_loge("SPI API type has to be 'spi' got '${arg_TYPE}'")
    endif()

    tbd_pinout_check("${arg_PINS}" PINS ${TBD_API_SPI_PINS})
endmacro()


# @brief constructor for SPI API
#
# @arg TYPE [enum]           has to be 'spi'
# @arg PINS [map<str,int>]   SPI pins
#
function (tbd_api_spi var_name)
    tbd_api_spi_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

function(tbd_api_spi_pins api_spi)
    tbd_api_spi_attrs(${api_spi})
    tbd_store_or_return("${arg_PINS}" ${ARGN})
endfunction()

function(tbd_api_spi_pin_flags api_spi)
    tbd_api_spi_attrs(${api_spi})
    tbd_pinout_flags("${arg_PINS}"
            PINS ${TBD_API_SPI_PINS}
            NAMESPACE TBD_API_SPI_PIN_
            PREFIX GPIO_NUM_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()


## methods ##

function(tbd_api_spi_print_info api_spi)
    tbd_api_spi_attrs(${api_spi})
    tbd_pinout_info("${arg_PINS}" PINS ${TBD_API_SPI_PINS} VAR pins)

    message("
TBD SPI API configuration
---------------------------
type: spi
${pins}---------------------------
    ")
endfunction()

function(_tbd_api_spi_load json_data)
    string(JSON pins_data GET "${json_data}" pins)
    tbd_pinout_load("${pins_data}"
            PINS ${TBD_API_SPI_PINS}
            VAR pins
    )

    set(new_api_spi
            PINS ${pins}
    )
    tbd_store_or_return("${new_api_spi}" ${ARGN})
endfunction()
