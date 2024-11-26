include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

set(TBD_CV_INPUT_TYPES adc mcp3208 stm32 midi)
set(TBD_CV_INPUT_GENERAL_ATTRS TYPE N_CVS N_TRIGGERS)

#### Adc cv_input class ####

# helper for accessing Adc fields
#
#
macro(tbd_adc_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for adc cv_input
#
# @arg TYPE [enum]   has to be 'adc'
#
function (tbd_adc var_name)
    tbd_adc_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "adc")
        tbd_loge("adc cv_input type has to be 'adc' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## adc methods ##

function(tbd_adc_print_info adc)
    message("
TBD cv_input configuration
---------------------------
type: adc
---------------------------
    ")
endfunction()



#### Mcp3208 cv_input class ####


# helper for accessing Mcp3208 fields
#
#
macro(tbd_mcp3208_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
        CS
        MOSI
        MISO
        SCLK
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for mcp3208 cv_input
#
# @arg TYPE [enum]   has to be 'mcp3208'
# @arg cs [int]      chip select pin
# @arg mosi [int]    SoC output pin
# @arg miso [int]    SoC input pin
# @arg sclk [int]    clock pin
#
function(tbd_mcp3208 var_name)
    tbd_mcp3208_attrs(${ARGN})
    if (NOT "${arg_TYPE}" STREQUAL "mcp3208")
        tbd_loge("mcp3208 cv_input type has to be 'mcp3208' got '${arg_TYPE}'")
    endif()

    tbd_check_int("${arg_CS}")
    tbd_check_int("${arg_MOSI}")
    tbd_check_int("${arg_MISO}")
    tbd_check_int("${arg_SCLK}")

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## mcp3208 properties ##

function(tbd_mcp3208_cs mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_store_or_return("${arg_CS}" ${ARGN})
endfunction()

function(tbd_mcp3208_mosi mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_store_or_return("${arg_MOSI}" ${ARGN})
endfunction()

function(tbd_mcp3208_miso mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_store_or_return("${arg_MISO}" ${ARGN})
endfunction()

function(tbd_mcp3208_sclk mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    tbd_store_or_return("${arg_SCLK}" ${ARGN})
endfunction()

## rbg methods ##

function(tbd_mcp3208_print_info mcp3208)
    tbd_mcp3208_attrs(${mcp3208})
    message("
TBD cv_input configuration
---------------------------
type: mcp3208

chip select pin: ${arg_CS}
output pin:      ${arg_MOSI}
input pin:       ${arg_MISO}
clock pin:       ${arg_SCLK}
---------------------------
    ")
endfunction()

function(_tbd_mcp3208_load json_data)
    tbd_logw("${json_data}")
    string(JSON type GET "${json_data}" type)
    string(JSON pins_data GET "${json_data}" pins)

    string(JSON cs GET "${pins_data}" cs)
    string(JSON mosi GET "${pins_data}" mosi)
    string(JSON miso GET "${pins_data}" miso)
    string(JSON sclk GET "${pins_data}" sclk)

    tbd_mcp3208(new_mcp3208 
        TYPE ${type}
        CS ${cs}    
        MOSI ${mosi}
        MISO ${miso}
        SCLK ${sclk}
    )
    tbd_store_or_return("${new_mcp3208}" ${ARGN})
endfunction()


#### Stm32 cv_input class ####

# helper for accessing Stm32 fields
#
#
macro(tbd_stm32_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for stm32 cv_input
#
# @arg TYPE [enum]   has to be 'stm32'
#
function (tbd_stm32 var_name)
    tbd_stm32_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "stm32")
        tbd_loge("stm32 cv_input type has to be 'stm32' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## stm32 methods ##

function(tbd_stm32_print_info stm32)
    message("
TBD cv_input configuration
---------------------------
type: stm32
---------------------------
    ")
endfunction()


function(tbd_stm32_load json_data)
    string(JSON type GET "${json_data}" type)
    tbd_stm32(new_stm32 
        TYPE ${type}
    )
    tbd_store_or_return("${new_stm32}" ${ARGN})
endfunction()


#### Midi cv_input class ####

# helper for accessing Midi fields
#
#
macro(tbd_midi_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "" ${ARGV})
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        tbd_loge("missing argument value for ${arg_KEYWORDS_MISSING_VALUES}")
    endif()
endmacro()


# @brief constructor for midi cv_input
#
# @arg TYPE [enum]   has to be 'midi'
#
function (tbd_midi var_name)
    tbd_midi_attrs(${ARGN})

    if (NOT "${arg_TYPE}" STREQUAL "midi")
        tbd_loge("midi cv_input type has to be 'midi' got '${arg_TYPE}'")
    endif()

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()

## midi methods ##

function(tbd_midi_print_info midi)
    message("
TBD cv_input configuration
---------------------------
type: midi
---------------------------
    ")
endfunction()


function(_tbd_midi_load json_data)
    tbd_store_or_return("" ${ARGN})
endfunction()


#### cv_input base class ####

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
    if ("${arg_TYPE}" STREQUAL "adc")
        tbd_adc_print_info("${cv_input}")
    elseif("${arg_TYPE}" STREQUAL "mcp3208")
        tbd_mcp3208_print_info("${cv_input}")
    elseif("${arg_TYPE}" STREQUAL "stm32")
        tbd_stm32_print_info("${cv_input}")
    elseif("${arg_TYPE}" STREQUAL "midi")
        tbd_midi_print_info("${cv_input}")
    else()
        message("invalid") 
    endif()
endfunction()

function(tbd_cv_input_load json_data)
    string(JSON type GET "${json_data}" type)

    if ("${type}" STREQUAL "adc")
        # tbd_adc_load("${json_data}" VAR specific)
    elseif("${type}" STREQUAL "mcp3208")
        _tbd_mcp3208_load("${json_data}" VAR specific)
    elseif("${type}" STREQUAL "stm32")
        # tbd_stm32_load("${json_data}" VAR specific)
    elseif("${type}" STREQUAL "midi")
        # tbd_midi_load("${json_data}" VAR specific)
    else()
        tbd_loge("unknown cv_input type '${type}' in json")
    endif()

    string(JSON n_cvs GET "${json_data}" n_cvs)
    string(JSON n_triggers GET "${json_data}" n_triggers)

    tbd_store_or_return("TYPE;${type};N_CVS;${n_cvs};N_TRIGGERS;${n_triggers};${specific}" ${ARGN})
endfunction()