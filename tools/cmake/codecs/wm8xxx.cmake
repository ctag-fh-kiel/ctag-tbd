## wm8xxx codec class ##

set(TBD_WM8XXX_SUBTYPES wm8731 wm8978 wm8974)
set(TBD_WM8XXX_SPI_PINS cs mosi miso clk)
set(TBD_WM8XXX_I2S_PINS bclk adcdat dacdat lrclk)


# helper for accessing wm8xxx fields
#
#
macro(tbd_wm8xxx_attrs)
    tbd_codec_attrs(${ARGN})
    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS};VOLUME_CONTROL" "SPI;I2S" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" IN_LIST TBD_WM8XXX_SUBTYPES)
        tbd_loge("wm8xxx codec type has to be 'wm8xxx' got '${arg_TYPE}'")
    endif()

    tbd_check_bool("${arg_VOLUME_CONTROL}")
    tbd_pinout_check("${arg_SPI}" PINS ${TBD_WM8XXX_SPI_PINS})
    tbd_pinout_check("${arg_I2S}" PINS ${TBD_WM8XXX_I2S_PINS})
endmacro()


# @brief constructor for wm8xxx
#
# @arg TYPE [enum]             has to be 'wm8xxx'
# @arg SPI [map<str,int>]      control communication pins
# @arg I2S [map<str,int>]      audio communication pins
# @arg VOLUME_CONTROL [bool]   keep ownership of control connection after initialization
#
function (tbd_wm8xxx var_name)
    tbd_wm8xxx_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

function(tbd_wm8xxx_volume_control wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_store_or_return("${arg_VOLUME_CONTROL}" ${ARGN})
endfunction()

function(tbd_wm8xxx_spi_pins wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_store_or_return("${arg_SPI}" ${ARGN})
endfunction()

function(tbd_wm8xxx_i2s_pins wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_store_or_return("${arg_I2S}" ${ARGN})
endfunction()

function(tbd_wm8xxx_type_flags wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_flag_list_flags("${arg_TYPE}"
            FLAGS ${TBD_WM8XXX_SUBTYPES}
            NAMESPACE TBD_AUDIO_
            VAR flags
    )
    tbd_store_or_return("TBD_AUDIO_WMXXX=1;${flags}" ${ARGN})
endfunction()

function(tbd_wm8xxx_pin_flags wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_pinout_flags("${arg_SPI}"
            PINS ${TBD_WM8XXX_SPI_PINS}
            NAMESPACE TBD_WM8XXX_SPI_PIN_
            PREFIX GPIO_NUM_
            VAR spi
    )
    tbd_pinout_flags("${arg_I2S}"
            PINS ${TBD_WM8XXX_I2S_PINS}
            NAMESPACE TBD_WM8XXX_I2S_PIN_
            PREFIX GPIO_NUM_
            VAR i2s
    )
    tbd_store_or_return("${spi};${i2s}" ${ARGN})
endfunction()



## methods ##

function(tbd_wm8xxx_print_info wm8xxx)
    tbd_wm8xxx_attrs(${wm8xxx})
    tbd_pinout_info("${arg_SPI}"
            PINS ${TBD_WM8XXX_SPI_PINS}
            VAR spi
    )
    tbd_pinout_info("${arg_I2S}"
            PINS ${TBD_WM8XXX_I2S_PINS}
            VAR i2s
    )
    message("
TBD codec configuration
---------------------------
type: ${arg_TYPE}

spi pins
........
${spi}
i2s pins
........
${i2s}---------------------------
    ")
endfunction()

function(_tbd_wm8xxx_load json_data)
    string(JSON pin_obj GET "${json_data}" pins)
    string(JSON volume_control GET "${json_data}" volume_control)
    tbd_to_bool(volume_control)

    string(JSON spi_data GET "${pin_obj}" spi)
    tbd_pinout_load("${spi_data}"
            PINS ${TBD_WM8XXX_SPI_PINS}
            VAR spi
    )

    string(JSON i2s_data GET "${pin_obj}" i2s)
    tbd_pinout_load("${i2s_data}"
            PINS ${TBD_WM8XXX_I2S_PINS}
            VAR i2s
    )

    set(new_wm8xxx
        VOLUME_CONTROL "${volume_control}"
        SPI "${spi}"
        I2S "${i2s}"
        WORK_TYPE pull
    )
    tbd_store_or_return("${new_wm8xxx}" ${ARGN})
endfunction()
