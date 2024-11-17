include(${PROJECT_DIR}/tools/cmake/platforms.cmake)

#### drivers setup ####

# activate drivers
#
# Make the drivers available to the entire project as `TBD_DRIVERS_OBJ`.
#
function(tbd_drivers_activate platform)
    tbd_drivers(drivers "${platform}")

    set(TBD_DRIVERS_OBJ "${drivers}" CACHE STRING "" FORCE)
    mark_as_advanced(TBD_DRIVERS_OBJ)

    tbd_drivers_print_info("${TBD_DRIVERS_OBJ}")
endfunction()


#### drivers class ####

# helper for accessing drivers fields
#
#
macro(tbd_drivers_attrs)
    set(file_lists
        SRCS
        ULP_SRCS
        INCLUDES
        LIBS
    )

    cmake_parse_arguments(arg "" "" "${file_lists}" ${ARGV})
endmacro()


#
#  constructor for drivers description
#
function(tbd_drivers var_name platform)
    tbd_platform_attrs(${platform})

    # gather input and trigger sources

    set(libs driver)

    if (DEFINED arg_CV_INPUT)
        if (arg_CV_INPUT STREQUAL adc)
            set(cv_input_srcs adc.cpp)
            set(triggers_srcs gpio.cpp)
            set(ulp_srcs ulp/adc.S adc.cpp)
            list(APPEND includes triggers)
            list(APPEND libs ulp)
        elseif (arg_CV_INPUT STREQUAL mcp3208)
            set(cv_input_srcs adc.cpp)
            set(ulp_srcs ulp/mcp3208.S adc.cpp)
            set(triggers_srcs gpio.cpp)
            list(APPEND includes triggers)
            list(APPEND libs ulp)
        elseif (arg_CV_INPUT STREQUAL stm32)
            set(cv_input_srcs mk2.cpp)
            # no GPIO triggers
            # no ULP
        elseif (arg_CV_INPUT STREQUAL midi)
            list(APPEND libs esp_tinyusb)
            set(cv_input_srcs  midiuart.cpp tusbmidi.cpp)
            # no GPIO triggers
            # no ULP
        else ()
            message("invalid CV input ${arg_CV_INPUT}")
        endif ()   

        list(APPEND includes cv_inputs)
        list(TRANSFORM cv_input_srcs PREPEND cv_inputs/)
        list(TRANSFORM ulp_srcs PREPEND cv_inputs/)
        list(TRANSFORM triggers_srcs PREPEND triggers/)
        list(APPEND srcs ${cv_input_srcs} ${triggers_srcs})
    else ()
        message("no CV input specified")
    endif ()

    # gather audio codec sources

    if (DEFINED arg_AUDIO_OUTPUT)
        if (arg_AUDIO_OUTPUT STREQUAL wm8xxx)
            set(codec_srcs codec.cpp)        
        elseif(arg_AUDIO_OUTPUT STREQUAL aic3254)
            set(codec_srcs codec_bba.cpp aic3254.cpp)
        elseif(arg_AUDIO_OUTPUT STREQUAL es8388)
            set(codec_srcs codec_bba.cpp es8388.cpp)
        else()
            message("invalid sound codec ${arg_AUDIO_OUTPUT}")
        endif()           

        list(APPEND includes codecs)
        list(TRANSFORM codec_srcs PREPEND codecs/)
        list(APPEND srcs ${codec_srcs})
    else()
        message("no sound codec specified")
    endif()  

    # gather indicator

    if (DEFINED arg_INDICATOR)
        if (arg_INDICATOR STREQUAL rgb)
            set(indicator_srcs led_rgb.cpp)   
        elseif(arg_INDICATOR STREQUAL neopixel)
            set(indicator_srcs led_rgb_bba.cpp)  
        else()
            message("invalid indicator type ${arg_INDICATOR}")
        endif()           

        list(APPEND includes indicator)
        list(TRANSFORM indicator_srcs PREPEND indicator/)
        list(APPEND srcs ${indicator_srcs})
    endif() 

    if (arg_FILE_SYSTEM)
        list(APPEND includes file_system)
        set(file_system_srcs fs.hpp fs.cpp)
        list(TRANSFORM file_system_srcs PREPEND file_system/)
        list(APPEND srcs ${file_system_srcs})
    endif()

    if (arg_DISPLAY)
        list(APPEND includes display)
        set(display_srcs ssd1306.c ssd1306_i2c.c)
        list(TRANSFORM display_srcs PREPEND display/)
        list(APPEND srcs ${display_srcs})
    endif()

    set(${var_name}
        SRCS ${srcs}
        ULP_SRCS "${ulp_srcs}"
        INCLUDES ${includes}
        LIBS ${libs}
        PARENT_SCOPE
    )
endfunction()

# drivers properties

function(tbd_drivers_sources drivers)
    tbd_drivers_attrs(${drivers})
    tbd_log("DRIVERS ${drivers}")
    tbd_log("SRCS ${arg_SRCS}")
    tbd_store_or_return("${arg_SRCS}" ${ARGN})
endfunction()

function(tbd_drivers_includes drivers)
    tbd_drivers_attrs(${drivers})
    tbd_store_or_return("${arg_INCLUDES}" ${ARGN})
endfunction()

function(tbd_drivers_libs drivers)
    tbd_drivers_attrs(${drivers})
    tbd_store_or_return("${arg_LIBS}" ${ARGN})
endfunction()

# drivers methods

function(tbd_drivers_embed_ulp drivers)
    tbd_drivers_attrs(${drivers})
    if (${arg_ULP_SRCS})
        set(ulp_app_name ulp_${COMPONENT_NAME})
        ulp_embed_binary(${ulp_app_name} ${$arg_ULP_SRCS})
    endif()
endfunction()

function(tbd_drivers_print_info drivers)
    tbd_drivers_attrs(${drivers})
message("
TBD drivers configuration
-------------------------
sources: ${arg_SRCS}
ULP sources: ${arg_ULP_SRCS}
include dirs: ${arg_INCLUDES}
dependencies: ${arg_LIBS}
-------------------------
    ")
endfunction()


### setup drivers library ####

tbd_drivers_activate("${TBD_PLATFORM_OBJ}")

tbd_log("${TBD_DRIVERS_OBJ}")
tbd_drivers_sources("${TBD_DRIVERS_OBJ}" VAR SRCS_FILES)
tbd_drivers_includes("${TBD_DRIVERS_OBJ}" VAR INCLUDE_DIRS)
tbd_drivers_libs("${TBD_DRIVERS_OBJ}" VAR LIBS)

idf_component_register(
    SRCS ${SRCS_FILES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES ${LIBS}
)

tbd_drivers_embed_ulp("${TBD_DRIVERS_OBJ}")
