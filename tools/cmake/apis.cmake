include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/pinouts.cmake)


if ("${TBD_API_TYPES}" STREQUAL "")
    file(GLOB api_files
        ${TBD_CMAKE_DIR}/apis/*.cmake
    )
    foreach(api_file ${api_files})
        get_filename_component(api_name "${api_file}" NAME_WLE)
        list(APPEND TBD_API_TYPES "${api_name}")
        include("${api_file}")
    endforeach()
endif()


#### api base class ####

# helper for accessing api fields
#
#
macro(tbd_api_attrs)
    cmake_parse_arguments(arg "" "TYPE" "" ${ARGV})

    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT arg_TYPE IN_LIST TBD_API_TYPES)
        tbd_loge("unknown api type '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for all api types
#
# @arg TYPE [enum]   type of api
#
function(tbd_api var_name)
    tbd_api_attrs(${ARGN})
    cmake_language(CALL "tbd_api_${arg_TYPE}" CHECK ${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## api properties ##

function(tbd_api_type api)
    tbd_api_attrs(${api})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

function(tbd_api_type_flags api)
    tbd_api_attrs(${api})

    tbd_flag_list_flags("${arg_TYPE}"
            FLAGS ${TBD_API_TYPES}
            NAMESPACE TBD_API_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## api methods ##

function(tbd_api_print_info api)
    tbd_api_attrs(${api})
    cmake_language(CALL "tbd_api_${arg_TYPE}_print_info" "${api}")
endfunction()

function(tbd_api_load json_data)
    string(JSON type GET "${json_data}" type)
    if (NOT type IN_LIST TBD_API_TYPES)
        tbd_loge("unknown api type '${type}'")
    endif()

    cmake_language(CALL "_tbd_api_${type}_load" "${json_data}" VAR specific)

    tbd_api(new_api
        TYPE "${type}"
        ${specific}
    )
    tbd_store_or_return("${new_api}" ${ARGN})
endfunction()