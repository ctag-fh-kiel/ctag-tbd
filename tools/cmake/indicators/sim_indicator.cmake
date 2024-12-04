## sim indicator class ##

# helper for accessing sim_indicator fields
#
#
macro(tbd_sim_indicator_attrs)
    tbd_indicator_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "sim_indicator")
        tbd_loge("sim_indicator indicator type has to be 'sim_indicator' got '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for sim_indicator indicators
#
# @arg TYPE [enum]   has to be 'sim_indicator'
#
function (tbd_sim_indicator var_name)
    tbd_sim_indicator_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## sim_indicator methods ##

function(tbd_sim_indicator_print_info sim_indicator)
    tbd_sim_indicator_attrs(${sim_indicator})
    message("
TBD indicator configuration
---------------------------
type: sim_indicator
---------------------------
    ")
endfunction()


function(_tbd_sim_indicator_load json_data)
    # no params
endfunction()