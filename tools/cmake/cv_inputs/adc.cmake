## Adc cv_input class ##

# helper for accessing Adc fields
#
#
macro(tbd_adc_attrs)
    set(attrs
            ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for adc cv_input
#
# @arg TYPE [enum]   has to be 'adc'
#
function (tbd_adc var_name)
    tbd_adc_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "adc")
        tbd_loge("adc cv_input type has to be 'adc' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## adc methods ##

function(tbd_adc_print_info adc)
    message("
TBD cv_input configuration
---------------------------
type: adc
---------------------------
    ")
endfunction()

function(_tbd_adc_load json_data)
    #    tbd_store_or_return("" ${ARGN})
endfunction()
