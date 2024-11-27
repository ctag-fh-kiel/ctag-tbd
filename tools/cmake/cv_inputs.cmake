include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/pinouts.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/flag_list.cmake)

file(GLOB cv_input_files
        ${CMAKE_CURRENT_LIST_DIR}/cv_inputs/*.cmake
)
foreach(cv_input_file ${cv_input_files})
    get_filename_component(cv_input_name "${cv_input_file}" NAME_WLE)
    list(APPEND TBD_CV_INPUT_TYPES "${cv_input_name}")
    include("${cv_input_file}")
endforeach()

## cv_input base class ##

set(TBD_CV_INPUT_GENERAL_ATTRS TYPE N_CVS N_TRIGGERS)

# helper for accessing cv_input fields
#
#
macro(tbd_cv_input_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()

# @brief constructor for all cv_input types
#
# @arg TYPE [enum]   type of cv_input
#
function (tbd_cv_input var_name)
    tbd_cv_input_attrs(${ARGN})

    if ("${arg_TYPE}" STREQUAL "adc")
        tbd_adc(CHECK ${ARGN})
    elseif("${arg_TYPE}" STREQUAL "mcp3208")
        tbd_mcp3208(CHECK ${ARGN})
    elseif("${arg_TYPE}" STREQUAL "stm32")
        tbd_stm32(CHECK ${ARGN})
    elseif("${arg_TYPE}" STREQUAL "midi")
        tbd_midi(CHECK ${ARGN})
    else()
        tbd_loge("unknown cv_input type ${arg_TYPE}")
    endif()

    tbd_check_int("${arg_N_CVS}")
    tbd_check_int("${arg_N_TRIGGERS}")

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)   
    endif()
endfunction()

## cv_input properties ##

function(tbd_cv_input_type cv_input)
    tbd_cv_input_attrs(${cv_input})
    tbd_store_or_return("${arg_TYPE}" ${ARGN})
endfunction()

function(tbd_cv_input_n_cvs cv_input)
    tbd_cv_input_attrs(${cv_input})
    tbd_store_or_return("${arg_N_CVS}" ${ARGN})
endfunction()

function(tbd_cv_input_n_triggers cv_input)
    tbd_cv_input_attrs(${cv_input})
    tbd_store_or_return("${arg_N_TRIGGERS}" ${ARGN})
endfunction()

function(tbd_cv_input_needs_calibration cv_input)
    tbd_cv_input_attrs(${cv_input})
    if ("${arg_TYPE}" STREQUAL "adc")
        tbd_store_or_return(yes ${ARGN})
    else()
        tbd_store_or_return(no ${ARGN})
    endif()
endfunction()

## cv_input methods ##

function(tbd_cv_input_print_info cv_input)
    tbd_cv_input_attrs(${cv_input})
    if (NOT ${arg_TYPE} IN_LIST TBD_CV_INPUT_TYPES)
        tbd_loge("unknown cv_input type '${arg_TYPE}'")
    endif()

    cmake_language(CALL "tbd_${arg_TYPE}_print_info" "${cv_input}")
endfunction()

function(tbd_cv_input_load json_data)
    string(JSON type GET "${json_data}" type)
    if (NOT ${type} IN_LIST TBD_CV_INPUT_TYPES)
        tbd_loge("unknown cv_input type '${arg_TYPE}'")
    endif()

    cmake_language(CALL "_tbd_${type}_load" "${json_data}" VAR specific)

    string(JSON n_cvs GET "${json_data}" n_cvs)
    string(JSON n_triggers GET "${json_data}" n_triggers)

    set(cv_input
            TYPE ${type}
            N_CVS ${n_cvs}
            N_TRIGGERS ${n_triggers}
            ${specific}
    )
    cmake_language(CALL "tbd_${type}" retval ${cv_input})
    tbd_store_or_return("${retval}" ${ARGN})
endfunction()