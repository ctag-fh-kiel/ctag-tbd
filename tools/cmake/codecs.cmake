include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/pinouts.cmake)

if ("${TBD_CODEC_TYPES}" STREQUAL "")
    file(GLOB codec_files
            ${CMAKE_CURRENT_LIST_DIR}/codecs/*.cmake
    )
    foreach(codec_file ${codec_files})
        get_filename_component(codec_name "${codec_file}" NAME_WLE)
        list(APPEND TBD_CODEC_TYPES "${codec_name}")
        include("${codec_file}")
    endforeach()
endif()


# associate a codec subtype with a broader device type
#
#
function(tbd_codec_map_to_group codec_type)
    if (codec_type IN_LIST TBD_WM8XXX_SUBTYPES)
        set(retval wm8xxx)
    else()
        set(retval "${codec_type}")
    endif()
    tbd_store_or_return("${retval}" ${ARGN})
endfunction()


#### codec base class ####

set(TBD_CODEC_WORK_TYPES pull push)

# helper for accessing codec fields
#
#
macro(tbd_codec_attrs)
    set(TBD_CODEC_GENERAL_ATTRS TYPE WORK_TYPE)

    cmake_parse_arguments(arg "" "${TBD_CODEC_GENERAL_ATTRS}" "" ${ARGV})
    tbd_codec_map_to_group(${arg_TYPE} VAR arg_DRIVER_TYPE)

    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    set(codec_types ${TBD_CODEC_TYPES})
    if (NOT arg_DRIVER_TYPE IN_LIST codec_types)
        tbd_loge("unknown codec type '${arg_DRIVER_TYPE}'")
    endif()

    if(NOT arg_WORK_TYPE IN_LIST TBD_CODEC_WORK_TYPES)
        TBD_LOGE("bad codec work type '${arg_WORK_TYPE}' has to be 'pull' or 'push'")
    endif()
endmacro()


# @brief constructor for all codec types
#
# @arg TYPE [enum]        exact chip or library name
# @arg WORK_TYPE [enum]   does the codec have to be polled 'pull' or call callbacks 'push'
#
function (tbd_codec var_name)
    tbd_codec_attrs(${ARGN})
    cmake_language(CALL "tbd_${arg_DRIVER_TYPE}" CHECK ${ARGN})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## codec properties ##

function(tbd_codec_type codec)
    tbd_codec_attrs(${codec})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

function(tbd_codec_work_type codec)
    tbd_codec_attrs(${codec})
    tbd_store_or_return("${arg_WORK_TYPE}" ${ARGN})
endfunction()

function(tbd_codec_work_type_flags codec)
    tbd_codec_attrs(${codec})
    tbd_flag_list_flags("${arg_WORK_TYPE}"
            FLAGS ${TBD_CODEC_WORK_TYPES}
            NAMESPACE TBD_AUDIO_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

## codec methods ##

function(tbd_codec_print_info codec)
    tbd_codec_attrs(${codec})
    cmake_language(CALL "tbd_${arg_DRIVER_TYPE}_print_info" "${codec}")
endfunction()

function(tbd_codec_load json_data)
    string(JSON type GET "${json_data}" type)
    tbd_codec_map_to_group(${type} VAR codec_type)
    if (NOT ${codec_type} IN_LIST TBD_CODEC_TYPES)
        tbd_loge("unknown codec type '${type}'")
    endif()

    cmake_language(CALL "_tbd_${codec_type}_load" "${json_data}" VAR specific)

    tbd_codec(new_codec
        TYPE "${type}"
        ${specific}
    )
    tbd_store_or_return("${new_codec}" ${ARGN})
endfunction()