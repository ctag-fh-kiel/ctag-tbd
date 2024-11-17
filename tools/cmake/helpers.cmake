function(tbd_set_define define value)
    add_compile_definitions(${define}=${value})

    tbd_is_idf()
    if (_return)
        idf_build_set_property(COMPILE_DEFINITIONS -D${define}=${value} APPEND)
    endif()
endfunction()

function(tbd_set_param var value)
    set(${var} ${value} CACHE STRING "" FORCE)
    mark_as_advanced(${var})

    tbd_is_idf()
    if (_return)
        idf_build_set_property(${var} ${value})
    endif()
endfunction()


macro(tbd_store_or_return value)
    cmake_parse_arguments(arg "" "VAR" "" ${ARGN})
    if (arg_VAR) 
        set(${arg_VAR} "${value}" PARENT_SCOPE)
    else()
        set(_return "${value}" PARENT_SCOPE)
    endif()
endmacro()


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
