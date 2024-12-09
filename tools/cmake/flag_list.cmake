include(${TBD_CMAKE_DIR}/helpers.cmake)


function(tbd_flag_list_load json_data)
    cmake_parse_arguments(arg "" "VAR" "FLAGS" ${ARGN})
    if (NOT arg_FLAGS)
        tbd_loge("no flag names provided")
    endif()

    string(JSON num_flags ERROR_VARIABLE _ LENGTH "${json_data}")

    foreach (i RANGE ${num_flags})
        if (i EQUAL num_flags)
            break()
        endif()

        string(JSON flag_type TYPE "${json_data}" ${i})
        if (NOT "${flag_type}" STREQUAL "STRING")
            tbd_loge("flag ${i} has bad type '${flag_type}'")
        endif()

        string(JSON flag GET "${json_data}" ${i})
        if (NOT flag IN_LIST arg_FLAGS)
            tbd_loge("unknown flag '${flag}', expected one of ${arg_FLAGS}")
        endif()

        if (flag IN_LIST flags)
            tbd_loge("duplicate flag '${flag}'")
        endif()

        list(APPEND flags ${flag})
    endforeach()
    tbd_store_or_return("${flags}" ${ARGV})
endfunction()


function(tbd_flag_list_check flags)
    cmake_parse_arguments(arg "" "VAR" "FLAGS" ${ARGN})
    if (NOT arg_FLAGS)
        tbd_loge("no flag names provided")
    endif()

    foreach(flag ${flags})
        if (NOT ${flag} IN_LIST arg_FLAGS)
            tbd_loge("unknown flag '${flag}', expected one of ${arg_FLAGS}")
        endif()
        list(LENGTH flags num_flags)
        list(REMOVE_DUPLICATES flags)
        list(LENGTH flags num_unique)
        if (NOT num_flags EQUAL num_unique)
            tbd_loge("duplicate flags in ${flags}")
        endif()
    endforeach()
endfunction()


function(tbd_flag_list_info flags)
    cmake_parse_arguments(arg "" "VAR" "FLAGS" ${ARGN})
    if (NOT arg_FLAGS)
        tbd_loge("no flag names provided")
    endif()

    set(info_str "")
    foreach(flag_name ${arg_FLAGS})
        if (flag_name IN_LIST flags)
            set(flag_value true)
        else()
            set(flag_value false)
        endif()

        string(APPEND info_str "${flag_name}: ${flag_value}\n")
    endforeach()
    tbd_store_or_return("${info_str}" ${ARGN})
endfunction()


function(tbd_flag_list_flags flags)
    cmake_parse_arguments(arg "" "VAR;PREFIX;NAMESPACE" "FLAGS" ${ARGN})
    if (NOT arg_FLAGS)
        tbd_loge("no flag names provided")
    endif()

    foreach(flag_name ${arg_FLAGS})
        if (flag_name IN_LIST flags)
            set(flag_value 1)
        else()
            set(flag_value 0)
        endif()
        string(TOUPPER "${flag_name}" flag_name)
        list(APPEND retval "${arg_NAMESPACE}${flag_name}=${arg_PREFIX}${flag_value}")
    endforeach()
    tbd_store_or_return("${retval}" ${ARGN})
endfunction()