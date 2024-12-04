## simulated CV inputs ##

# helper for accessing Adc fields
#
#
macro(tbd_sim_cv_attrs)
    tbd_cv_input_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for adc sim_cv
#
# @arg TYPE [enum]   has to be 'adc'
#
function (tbd_sim_cv var_name)
    tbd_sim_cv_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "sim_cv")
        tbd_loge("sim_cv input type has to be 'sim_cv' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## adc methods ##

function(tbd_sim_cv_print_info adc)
    message("
TBD sim_cv configuration
---------------------------
type: sim_cv
---------------------------
    ")
endfunction()

function(_tbd_sim_cv_load json_data)
    #    tbd_store_or_return("" ${ARGN})
endfunction()
