include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)


function(tbd_pinout_load json_data)
    cmake_parse_arguments(arg "" "VAR" "PINS" ${ARGN})
    if (NOT arg_PINS)
        tbd_loge("no pin names provided")
    endif()

    foreach(pin_name ${arg_PINS})
        string(JSON type ERROR_VARIABLE _ TYPE "${json_data}" "${pin_name}")

        if ("${type}" MATCHES ".*-NOTFOUND$")
            tbd_loge("error expected pin '${pin_name}' in JSON")
        endif()

        if (NOT "${type}" STREQUAL "NUMBER")
            tbd_loge("error pin '${pin_name}' has to be a number")
        endif()

        string(JSON value ERROR_VARIABLE err GET "${json_data}" "${pin_name}")
        list(APPEND pins "${pin_name}" "${value}")
    endforeach()

    tbd_store_or_return("${pins}" ${ARGV})
endfunction()


function(tbd_pinout_check pins)
    cmake_parse_arguments(arg "" "VAR" "PINS" ${ARGN})
    if (NOT arg_PINS)
        tbd_loge("no pin names provided")
    endif()

    cmake_parse_arguments(pin_map "" "${arg_PINS}" "" ${pins})
    if (DEFINED pin_map_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing pin value ${pin_map_KEYWORDS_MISSING_VALUES}")
    endif()

    foreach(pin_name ${arg_PINS})
        set(pin_value "${pin_map_${pin_name}}")
        if (NOT pin_value STREQUAL -1)
            tbd_check_int("${pin_value}" ERR err)
        endif()
        if(err)
            tbd_loge("pin '${pin_name}' value has to be a number, got ${pin_value}")
        endif()
    endforeach()
endfunction()


function(tbd_pinout_info pins)
    cmake_parse_arguments(arg "" "VAR" "PINS" ${ARGN})
    if (NOT arg_PINS)
        tbd_loge("no pin names provided")
    endif()

    cmake_parse_arguments(pin_map "" "${arg_PINS}" "" ${pins})
    if (DEFINED pin_map_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing pin value ${pin_map_KEYWORDS_MISSING_VALUES}")
    endif()

    set(info_str "")
    foreach(pin_name ${arg_PINS})
        set(pin_value "${pin_map_${pin_name}}")
        string(APPEND info_str "${pin_name} pin: ${pin_value}\n")
    endforeach()
    tbd_store_or_return("${info_str}" ${ARGN})
endfunction()


function(tbd_pinout_flags pins)
    cmake_parse_arguments(arg "" "VAR;PREFIX;NAMESPACE" "PINS" ${ARGN})
    if (NOT arg_PINS)
        tbd_loge("no pin names provided")
    endif()

    cmake_parse_arguments(pin_map "" "${arg_PINS}" "" ${pins})
    if (DEFINED pin_map_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing pin value ${pin_map_KEYWORDS_MISSING_VALUES}")
    endif()

    set(flags "")
    foreach(pin_name ${arg_PINS})
        set(pin_value "${pin_map_${pin_name}}")
        string(TOUPPER "${pin_name}" pin_name)
        if (pin_value STREQUAL -1)
            list(APPEND flags "${arg_NAMESPACE}${pin_name}=-1")
        else()
            list(APPEND flags "${arg_NAMESPACE}${pin_name}=${arg_PREFIX}${pin_value}")
        endif()
    endforeach()
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()