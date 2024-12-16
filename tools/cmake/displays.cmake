include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/pinouts.cmake)

if ("${TBD_DISPLAY_TYPES}" STREQUAL "")
    file(GLOB display_files
        ${TBD_CMAKE_DIR}/displays/*.cmake
    )
    foreach(display_file ${display_files})
        get_filename_component(display_name "${display_file}" NAME_WLE)
        list(APPEND TBD_DISPLAY_TYPES "${display_name}")
        include("${display_file}")
    endforeach()
endif()


#### display base class ####

# helper for accessing display fields
#
#
macro(tbd_display_attrs)
    cmake_parse_arguments(arg "" "TYPE" "" ${ARGV})

    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT arg_TYPE IN_LIST TBD_DISPLAY_TYPES)
        tbd_loge("unknown display type '${arg_TYPE}'")
    endif()
endmacro()


# @brief constructor for all display types
#
# @arg TYPE [enum]   type of display
#
function (tbd_display var_name)
    tbd_display_attrs(${ARGN})
    cmake_language(CALL "tbd_${arg_TYPE}" CHECK ${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## display properties ##

function(tbd_display_type display)
    tbd_display_attrs(${display})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

function(tbd_display_type_flags display)
    tbd_display_attrs(${display})

    tbd_flag_list_flags("${arg_TYPE}"
        FLAGS ${TBD_DISPLAY_TYPES}
        NAMESPACE TBD_DISPLAY_
        VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## display methods ##

function(tbd_display_print_info display)
    tbd_display_attrs(${display})
    cmake_language(CALL "tbd_${arg_TYPE}_print_info" "${display}")
endfunction()

function(tbd_display_load json_data)
    string(JSON type GET "${json_data}" type)
    if (NOT type IN_LIST TBD_DISPLAY_TYPES)
        tbd_loge("unknown display type '${type}'")
    endif()

    cmake_language(CALL "_tbd_${type}_load" "${json_data}" VAR specific)

    tbd_display(new_display
        TYPE "${type}"
        ${specific}
    )
    tbd_store_or_return("${new_display}" ${ARGN})
endfunction()