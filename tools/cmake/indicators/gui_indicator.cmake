## gui_indicator indicator class ##

# helper for accessing gui_indicator fields
#
#
macro(tbd_gui_indicator_attrs)
    tbd_indicator_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "gui_indicator")
        tbd_loge("gui_indicator indicator type has to be 'gui_indicator' got '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for gui_indicator indicators
#
# @arg TYPE [enum]   has to be 'gui_indicator'
#
function (tbd_gui_indicator var_name)
    tbd_gui_indicator_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## gui_indicator methods ##

function(tbd_gui_indicator_print_info gui_indicator)
    tbd_gui_indicator_attrs(${gui_indicator})
    message("
TBD indicator configuration
---------------------------
type: gui_indicator
---------------------------
    ")
endfunction()


function(_tbd_gui_indicator_load json_data)
    # no params
endfunction()