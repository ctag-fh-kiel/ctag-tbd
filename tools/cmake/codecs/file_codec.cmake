## file codec class ##

# helper for accessing file_codec fields
#
#
macro(tbd_file_codec_attrs)
    tbd_codec_attrs(${ARGV})

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for file_codec indicators
#
# @arg TYPE [enum]   has to be 'file_codec'
#
function (tbd_file_codec var_name)
    tbd_file_codec_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "file_codec")
        tbd_loge("file_codec indicator type has to be 'file_codec' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## file_codec methods ##

function(tbd_file_codec_print_info file_codec)
    tbd_file_codec_attrs(${file_codec})
    message("
TBD indicator configuration
---------------------------
type: file_codec
---------------------------
    ")
endfunction()


function(_tbd_file_codec_load json_data)
    tbd_store_or_return("WORK_TYPE;pull" ${ARGN})
endfunction()