include(${TBD_CMAKE_DIR}/helpers.cmake)
include(${TBD_CMAKE_DIR}/pinouts.cmake)
include(${TBD_CMAKE_DIR}/flag_list.cmake)

if ("${TBD_CV_INPUT_TYPES}" STREQUAL "")
    file(GLOB cv_input_files
            ${CMAKE_CURRENT_LIST_DIR}/cv_inputs/*.cmake
    )
    foreach(cv_input_file ${cv_input_files})
        get_filename_component(cv_input_name "${cv_input_file}" NAME_WLE)
        list(APPEND TBD_CV_INPUT_TYPES "${cv_input_name}")
        include("${cv_input_file}")
    endforeach()
endif()


## cv_input base class ##

set(TBD_CV_INPUT_GENERAL_ATTRS TYPE N_CVS N_TRIGGERS)

# helper for accessing cv_input fields
#
#
macro(tbd_cv_input_attrs)
    cmake_parse_arguments(arg "" "${TBD_CV_INPUT_GENERAL_ATTRS}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()

    if (NOT arg_TYPE IN_LIST TBD_CV_INPUT_TYPES)
        tbd_loge("unknown cv_input type '${arg_TYPE}'")
    endif()

    tbd_check_int("${arg_N_CVS}")
    tbd_check_int("${arg_N_TRIGGERS}")
endmacro()

# @brief constructor for all cv_input types
#
# @arg TYPE [enum]   type of cv_input
#
function (tbd_cv_input var_name)
    tbd_cv_input_attrs(${ARGN})
    cmake_language(CALL "tbd_${arg_TYPE}" CHECK ${ARGN})

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
    if ("${arg_TYPE}" STREQUAL "adc" OR "${arg_TYPE}" STREQUAL "mcp3208" )
        tbd_store_or_return(true ${ARGN})
    else()
        tbd_store_or_return(false ${ARGN})
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
        tbd_loge("unknown cv_input type '${type}'")
    endif()

    string(JSON n_cvs GET "${json_data}" n_cvs)
    string(JSON n_triggers GET "${json_data}" n_triggers)

    cmake_language(CALL "_tbd_${type}_load" "${json_data}" VAR specific)

    tbd_cv_input(new_cv_input
            TYPE ${type}
            N_CVS ${n_cvs}
            N_TRIGGERS ${n_triggers}
            ${specific}
    )
    tbd_store_or_return("${new_cv_input}" ${ARGN})
endfunction()