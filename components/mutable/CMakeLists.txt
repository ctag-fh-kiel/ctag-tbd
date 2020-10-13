# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
        eurorack/plaits/dsp/*.cc
        eurorack/plaits/dsp/engine/*.cc
        eurorack/plaits/dsp/speech/*.cc
        eurorack/plaits/dsp/physical_modelling/*.cc
        eurorack/plaits/resources.cc
        eurorack/plaits/dsp/*.h
        eurorack/plaits/dsp/engine/*.h
        eurorack/plaits/dsp/speech/*.h
        eurorack/plaits/dsp/physical_modelling/*.h
        eurorack/plaits/resources.h
        eurorack/rings/dsp/fm_voice.cc
        eurorack/rings/dsp/part.cc
        eurorack/rings/dsp/string_synth_part.cc
        eurorack/rings/dsp/string.cc
        eurorack/rings/dsp/resonator.cc
        eurorack/rings/resources.cc
        eurorack/tides2/poly_slope_generator.cc
        eurorack/tides2/ramp_extractor.cc
        eurorack/tides2/resources.cc
        eurorack/clouds/resources.cc
        eurorack/stmlib/utils/random.cc
        eurorack/stmlib/utils/random.h
        eurorack/stmlib/dsp/atan.cc
        eurorack/stmlib/dsp/units.cc
        eurorack/stmlib/dsp/dsp.h
        eurorack/braids/svf.h
        eurorack/braids/resources.cc
        eurorack/braids/resources.h
        eurorack/braids/analog_oscillator.cc
        eurorack/braids/analog_oscillator.h
        eurorack/braids/signature_waveshaper.h
        eurorack/braids/digital_oscillator.h
        eurorack/braids/digital_oscillator.cc
        eurorack/braids/macro_oscillator.h
        eurorack/braids/macro_oscillator.cc
        eurorack/braids/excitation.h
        eurorack/braids/quantizer.cc
        eurorack/braids/quantizer.h
        eurorack/braids/quantizer_scales.h
        )

if(TBD_SIM)
    add_library(mutable STATIC ${SRCS_FILES})
    target_include_directories(mutable PRIVATE "eurorack")
    target_compile_options(mutable PRIVATE
            -Wno-unused-variable
            -Wno-shift-negative-value
            )

else()
    idf_component_register(SRCS ${SRCS_FILES}
            INCLUDE_DIRS eurorack
            PRIV_REQUIRES )

    target_compile_options(${COMPONENT_LIB} PRIVATE
            -Wno-unused-variable
            -Wno-shift-negative-value
            )
endif()