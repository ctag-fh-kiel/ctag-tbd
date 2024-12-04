## sim codec class ##

# helper for accessing sim_codec fields
#
#
macro(tbd_sim_codec_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for sim_codec indicators
#
# @arg TYPE [enum]   has to be 'sim_codec'
#
function (tbd_sim_codec var_name)
    tbd_sim_codec_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "sim_codec")
        tbd_loge("sim_codec indicator type has to be 'sim_codec' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## sim_codec methods ##

function(tbd_sim_codec_print_info sim_codec)
    tbd_sim_codec_attrs(${sim_codec})
    message("
TBD indicator configuration
---------------------------
type: sim_codec
---------------------------
    ")
endfunction()


function(_tbd_sim_codec_load json_data)
    tbd_store_or_return("WORK_TYPE;pull" ${ARGN})
endfunction()