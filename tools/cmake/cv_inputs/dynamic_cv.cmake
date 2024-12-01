## run time switchable CV input ##

# helper for accessing Adc fields
#
#
macro(tbd_dynamic_cv_attrs)
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
function (tbd_dynamic_cv var_name)
    tbd_dynamic_cv_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "dynamic_cv")
        tbd_loge("dynamic_cv input type has to be 'dynamic_cv' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## adc methods ##

function(tbd_dynamic_cv_print_info adc)
    message("
TBD cv_input configuration
---------------------------
type: dynamic_cv
---------------------------
    ")
endfunction()

function(_tbd_dynamic_cv_load json_data)
    #    tbd_store_or_return("" ${ARGN})
endfunction()
