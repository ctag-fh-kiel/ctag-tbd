## serial API class ##

# helper for accessing REST API fields
#
#
macro(tbd_api_serial_attrs)
    tbd_api_attrs(${ARGV})

    cmake_parse_arguments(arg "" "" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT "${arg_TYPE}" STREQUAL "serial")
        tbd_loge("serial API type has to be 'serial' got '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for serial API
#
# @arg TYPE [enum]    has to be 'serial'
#
function(tbd_api_serial var_name)
    tbd_api_serial_attrs(${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## properties ##

# no properties

## methods ##

function(tbd_api_serial_print_info api_serial)
    tbd_api_serial_attrs(${api_serial})

    message("
TBD serial API configuration
---------------------------
type: serial
---------------------------
    ")
endfunction()

function(_tbd_api_serial_load json_data)
    tbd_store_or_return("" ${ARGN})
endfunction()
