## hardware MIDI CV input ##

set(TBD_MIDI_INPUT_TYPES uart usb rp2040 rtmidi)

# helper for accessing Midi fields
#
#
macro(tbd_midi_attrs)
    set(attrs
        ${TBD_CV_INPUT_GENERAL_ATTRS}
    )
    cmake_parse_arguments(arg "" "${attrs}" "MIDI_INPUTS" ${ARGV})
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

    tbd_flag_list_check("${arg_MIDI_INPUTS}" FLAGS ${TBD_MIDI_INPUT_TYPES})

    if (NOT "${var_name}" STREQUAL "CHECK")
        set(${var_name} ${ARGN} PARENT_SCOPE)
    endif()
endfunction()


## midi methods ##

function(tbd_midi_inputs_flags midi)
    tbd_midi_attrs(${midi})
    tbd_flag_list_flags("${arg_MIDI_INPUTS}"
            FLAGS ${TBD_MIDI_INPUT_TYPES}
            NAMESPACE TBD_MIDI_INPUT_
            VAR flags
    )
    tbd_store_or_return("${flags}" ${ARGN})
endfunction()

function(tbd_midi_print_info midi)
    tbd_midi_attrs(${midi})
    tbd_flag_list_info("${arg_MIDI_INPUTS}" FLAGS ${TBD_MIDI_INPUT_TYPES} VAR flags)
    message("
TBD cv_input configuration
---------------------------
type: midi
${flags}---------------------------
    ")
endfunction()


function(_tbd_midi_load json_data)
    string(JSON type GET "${json_data}" type)
    string(JSON inputs_json GET "${json_data}" inputs)

    tbd_flag_list_load("${inputs_json}" FLAGS ${TBD_MIDI_INPUT_TYPES} VAR inputs)

    set(new_midi
        MIDI_INPUTS ${inputs}
    )

    tbd_store_or_return("${new_midi}" ${ARGN})
endfunction()
