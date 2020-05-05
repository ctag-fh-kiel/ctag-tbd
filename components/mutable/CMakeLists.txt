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
        eurorack/stmlib/utils/random.cc
        eurorack/stmlib/dsp/atan.cc
        eurorack/stmlib/dsp/units.cc
        eurorack/stmlib/dsp/dsp.h
        )

idf_component_register(SRCS ${SRCS_FILES}
        INCLUDE_DIRS eurorack
        PRIV_REQUIRES )
