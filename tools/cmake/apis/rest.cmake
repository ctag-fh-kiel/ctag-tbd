## REST API class ##

# helper for accessing REST API fields
#
#
macro(tbd_api_rest_attrs)
    tbd_api_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "rest")
        tbd_loge("REST API type has to be 'rest' got '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for REST API
#
# @arg TYPE [enum]           has to be 'rest'
#
function(tbd_api_rest var_name)
    tbd_api_rest_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

# no properties

## methods ##

function(tbd_api_rest_print_info api_rest)
    tbd_api_rest_attrs(${api_rest})

    message("
TBD REST API configuration
---------------------------
type: rest
---------------------------
    ")
endfunction()

function(_tbd_api_rest_load json_data)
    tbd_store_or_return("" ${ARGN})
endfunction()
