## rtaudio output class ##

# helper for accessing rtaudio fields
#
#
macro(tbd_rtaudio_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for rtaudio indicators
#
# @arg TYPE [enum]   has to be 'rtaudio'
#
function (tbd_rtaudio var_name)
    tbd_rtaudio_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "rtaudio")
        tbd_loge("rtaudio indicator type has to be 'rtaudio' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## rtaudio methods ##

function(tbd_rtaudio_print_info rtaudio)
    tbd_rtaudio_attrs(${rtaudio})
    message("
TBD indicator configuration
---------------------------
type: rtaudio
---------------------------
    ")
endfunction()


function(_tbd_rtaudio_load json_data)
    tbd_store_or_return("WORK_TYPE;push" ${ARGN})
endfunction()