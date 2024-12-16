# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
        eurorack/plaits/dsp/*.h
        eurorack/plaits/dsp/*.cc
        eurorack/plaits/dsp/chords/*.cc
        eurorack/plaits/dsp/downsampler/*.h
        eurorack/plaits/dsp/drums/*.h
        eurorack/plaits/dsp/engine/*.h
        eurorack/plaits/dsp/engine/*.cc
        eurorack/plaits/dsp/engine2/*.cc
        eurorack/plaits/dsp/fm/*.cc
        eurorack/plaits/dsp/fm/*.h
        eurorack/plaits/dsp/fx/*.cc
        eurorack/plaits/dsp/noise/*.h
        eurorack/plaits/dsp/oscillator/*.cc
        eurorack/plaits/dsp/oscillator/*.h
        eurorack/plaits/dsp/speech/*.cc
        eurorack/plaits/dsp/speech/*.h
        eurorack/plaits/dsp/physical_modelling/*.h
        eurorack/plaits/dsp/physical_modelling/*.cc
        eurorack/plaits/resources.cc
        eurorack/plaits/resources.h
        eurorack/rings/dsp/fm_voice.cc
        eurorack/rings/dsp/part.cc
        eurorack/rings/dsp/string_synth_part.cc
        eurorack/rings/dsp/string.cc
        eurorack/rings/dsp/resonator.cc
        eurorack/rings/resources.cc
        eurorack/tides2/poly_slope_generator.cc
        eurorack/tides2/poly_slope_generator.h
        eurorack/tides2/ramp/ramp_extractor.cc
        eurorack/tides2/ramp/ramp_extractor.h
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
        eurorack/clouds/resources.cc
        eurorack/clouds/resources.h
        eurorack/clouds/dsp/audio_buffer.h
        eurorack/clouds/dsp/correlator.cc
        eurorack/clouds/dsp/correlator.h
        eurorack/clouds/dsp/frame.h
        eurorack/clouds/dsp/grain.h
        eurorack/clouds/dsp/granular_processor.cc
        eurorack/clouds/dsp/granular_processor.h
        eurorack/clouds/dsp/granular_sample_player.h
        eurorack/clouds/dsp/looping_sample_player.h
        eurorack/clouds/dsp/mu_law.cc
        eurorack/clouds/dsp/mu_law.h
        eurorack/clouds/dsp/parameters.h
        eurorack/clouds/dsp/sample_rate_converter.h
        eurorack/clouds/dsp/window.h
        eurorack/clouds/dsp/wsola_sample_player.h
        eurorack/clouds/dsp/fx/diffuser.h
        eurorack/clouds/dsp/fx/fx_engine.h
        eurorack/clouds/dsp/fx/pitch_shifter.h
        eurorack/clouds/dsp/fx/reverb.h
        eurorack/clouds/dsp/pvoc/frame_transformation.cc
        eurorack/clouds/dsp/pvoc/frame_transformation.h
        eurorack/clouds/dsp/pvoc/phase_vocoder.cc
        eurorack/clouds/dsp/pvoc/phase_vocoder.h
        eurorack/clouds/dsp/pvoc/stft.cc
        eurorack/clouds/dsp/pvoc/stft.h
)

# __idf_esp_system

add_library(mutable STATIC ${SRCS_FILES})
target_include_directories(mutable PUBLIC eurorack)
target_link_libraries(mutable 
        PRIVATE tbd_helpers)
target_compile_options(mutable PRIVATE
        -Wno-unused-variable
        -Wno-shift-negative-value
)
target_compile_definitions(mutable PUBLIC MUTABLE_STD_CPP=1)
