# @brief convert a subset of CMake's bools into true/false representation
#
# Accepted values are
# - '1' and '0'
# - 'true' and 'false'
# - 'yes' and 'no'
# - 'y' and 'n'
# - 'on' and 'off'
#
# @note: This is not the full set of truthful values CMake supports.
#        Only 'true booleans' are supported.
#
# @arg value [any]   value to convert
#
# @exception FATAL_ERROR
#
function (tbd_to_bool var_name)
    set(value "${${var_name}}")
    set(VALID_BOOLS 0 1 true false yes no y n on off)
    string(TOLOWER "${value}" lower_value)
    if(NOT lower_value IN_LIST VALID_BOOLS)
        TBD_LOGE("bad boolean value '${lower_value}' must be one of ${VALID_BOOLS}")
    endif()

    if (lower_value)
        set(normalized_value true)
    else()
        set(normalized_value false)
    endif()

    set("${var_name}" "${normalized_value}" PARENT_SCOPE)
endfunction()

# @brief raise an error if value is a boolean
#
# @note: Unlike CMake's default, we are restricting booleans to 'true' and 'false'
#        for more consistency.
#
# @arg value [any]   value to be checked
#
# @exception FATAL_ERROR
#
function (tbd_check_bool value)
    cmake_parse_arguments(arg "" "ERR" "" ${ARGN})
    string(STRIP "${value}" stripped_value)
    if (NOT (stripped_value STREQUAL true OR stripped_value STREQUAL false))
        set(retval no)
    else()
        set(retval true)
    endif()

    if (retval)
        set("${arg_ERR}" true)
    else()
        tbd_loge("expected boolean got ${value}")
    endif()
endfunction()

# @brief raise an error if value is not an integer
#
# @arg value [any]   value to be checked
#
# @exception FATAL_ERROR
#
function (tbd_check_int value)
    cmake_parse_arguments(arg "" "ERR" "" ${ARGN})
    string(STRIP "${value}" stripped_value)
    if (NOT stripped_value MATCHES "^[0-9]+$")
        set(retval no)
    else()
        set(retval true)
    endif()

    if (retval)
        set("${arg_ERR}" true)
    else()
        tbd_loge("expected integer got ${value}")
    endif()
endfunction()

# @brief set global parameter
#
# Set a global CMake variable visible in all parts of the build configuration:
#
# - plain CMake build configuration
# - ESP IDF build configuration
#
# @note: the variable is cached
#
function(tbd_set_param var value)
    set(${var} ${value} CACHE STRING "" FORCE)
    mark_as_advanced(${var})

    if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
        idf_build_set_property(${var} "${value}")
    endif()
endfunction()


# @brief set global parameter with compile time visibility
#
# Like `tbd_set_param` with additional visibility in compilation. The variable is 
# available in
#
# - plain CMake build configuration
# - ESP IDF build configuration
# - compilation (as global define)
#
# @note: the variable is cached
#
function(tbd_set_compile_param var value)
    tbd_set_param($var "$value")
    add_compile_definitions("${var}=${value}")

    # if ("${TBD_TOOLCHAIN}" STREQUAL "esp32")
    #     idf_build_set_property(COMPILE_DEFINITIONS -D${define}=${value} APPEND)
    # endif()
endfunction()


# read a value from config file
#
# Config files should contain simple key-value pairs. Each line contains a single 
# 
#    key=value 
#
# Additionally empty lines, comment lines and lines with with trailing comments 
# using `#` are allowed:
#
#    # I am a comment line
#
#    # there was an empty line here
#    key = value # this is a trailing comment
#
# @arg file [path]       path to config file
# @arg var [str]         name of the config value
#
# @return [none | str]   returns either value or nothing if value key is not in file
#
function(tbd_get_from_config_file file var)
    file(STRINGS "${file}" file_lines)
    string(REGEX MATCH ";\\s*${var}+\\s*=\\s*([^#\\s;]*)" value "${file_lines}")
    if (NOT "$value" STREQUAL "")
        tbd_store_or_return("${CMAKE_MATCH_1}" ${ARGN})
    endif()
endfunction()


# @brief function decorator for return values
#
# Returns a value from a function as a variable in the parent (caller) scope.
#
# There is two different use cases:
# 1. plain return: just return to `_return`
# 2. named returns: decorate the function to allow either default `_return` or setting a 
#    desired return name via the `VAR` parameter.
#
# @note: the possible third case of 'named returns only' is not intended.
#
#
# Using for plain returns:
#
# If `tbd_store_or_return` is called with only a value argument it will always set the 
# default `_return` variable in the parent (caller) scope.
#
#
# Using for plain returns and named returns:
# 
# Add `VAR` argument to parent to allow the following:
#
#    some_func(... VAR <output-name> ...)
#
# Pass unevaluated function arguments `NARG` to `tbd_store_or_return`, to achieve the
# following behavior:
#
# - function was not called with a named argument `VAR`: return value as default `_return`
# - function called with `VAR` argument: return value with this name.
#
# @arg value [any]   value to be returned
# @args VARGS        all non explicit function arguments
#
macro(tbd_store_or_return value)
    cmake_parse_arguments(_arg "" "VAR" "" ${ARGN})
    if (_arg_VAR)
        set(${_arg_VAR} "${value}" PARENT_SCOPE)
    else()
        set(_return "${value}" PARENT_SCOPE)
    endif()
endmacro()



# @brief raise an error if last function call returned nothing
#
#
macro(tbd_check_return)
    if ("${_return}" STREQUAL "")
        tbd_loge("function returned NULL! ${argv}")
    endif()
endmacro()


macro(tbd_logv)
   message(VERBOSE "[TBD DEBUG] ${ARGN}")
endmacro()


macro(tbd_log)
   message("[TBD MESSAGE] ${ARGN}")
endmacro()


macro(tbd_logw)
    message(WARNING "[TBD WARNING] ${ARGN}")
endmacro()


macro(tbd_loge)
    message(FATAL_ERROR "[TBD ERROR] ${ARGN}")
endmacro()
