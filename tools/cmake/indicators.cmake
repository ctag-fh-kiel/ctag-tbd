include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/pinouts.cmake)

if ("${TBD_INDICATOR_TYPES}" STREQUAL "")
    file(GLOB indicator_files
        ${TBD_CMAKE_DIR}/indicators/*.cmake
    )
    foreach(indicator_file ${indicator_files})
        get_filename_component(indicator_name "${indicator_file}" NAME_WLE)
        list(APPEND TBD_INDICATOR_TYPES "${indicator_name}")
        include("${indicator_file}")
    endforeach()
endif()


#### indicator base class ####

# helper for accessing indicator fields
#
#
macro(tbd_indicator_attrs)
    cmake_parse_arguments(arg "" "TYPE" "" ${ARGV})

    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT arg_TYPE IN_LIST TBD_INDICATOR_TYPES)
        tbd_loge("unknown indicator type '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for all indicator types
#
# @arg TYPE [enum]   type of indicator
#
function (tbd_indicator var_name)
    tbd_indicator_attrs(${ARGN})
    cmake_language(CALL "tbd_${arg_TYPE}" CHECK ${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## indicator properties ##

function(tbd_indicator_type indicator)
    tbd_indicator_attrs(${indicator})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

function(tbd_indicator_type_flags indicator)
    tbd_indicator_attrs(${indicator})

    tbd_flag_list_flags("${arg_TYPE}"
            FLAGS ${TBD_INDICATOR_TYPES}
            NAMESPACE TBD_INDICATOR_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## indicator methods ##

function(tbd_indicator_print_info indicator)
    tbd_indicator_attrs(${indicator})
    cmake_language(CALL "tbd_${arg_TYPE}_print_info" "${indicator}")
endfunction()

function(tbd_indicator_load json_data)
    string(JSON type GET "${json_data}" type)
    if (NOT type IN_LIST TBD_INDICATOR_TYPES)
        tbd_loge("unknown indicator type '${type}'")
    endif()

    cmake_language(CALL "_tbd_${type}_load" "${json_data}" VAR specific)

    tbd_indicator(new_indicator
        TYPE "${type}"
        ${specific}
    )
    tbd_store_or_return("${new_indicator}" ${ARGN})
endfunction()