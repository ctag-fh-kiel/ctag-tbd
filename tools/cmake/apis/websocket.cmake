## REST API class ##

# helper for accessing API_WEBSOCKET fields
#
#
macro(tbd_api_websocket_attrs)
    tbd_api_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "websocket")
        tbd_loge("websocket API type has to be 'websocket' got '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for websocket API
#
# @arg TYPE [enum]           has to be 'websocket'
#
function (tbd_api_websocket var_name)
    tbd_api_websocket_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

# no properties

## methods ##

function(tbd_api_websocket_print_info api_websocket)
    tbd_api_websocket_attrs(${api_websocket})

    message("
TBD websocket API configuration
---------------------------
type: websocket
---------------------------
    ")
endfunction()

function(_tbd_api_websocket_load json_data)
    tbd_store_or_return("" ${ARGN})
endfunction()
