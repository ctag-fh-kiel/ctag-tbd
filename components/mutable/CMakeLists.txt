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
        eurorack/stmlib/utils/random.cc
        eurorack/stmlib/dsp/atan.cc
        eurorack/stmlib/dsp/units.cc
        )

idf_component_register(SRCS ${SRCS_FILES}
        INCLUDE_DIRS eurorack
        PRIV_REQUIRES )

target_compile_options(${COMPONENT_LIB} PRIVATE -DTEST)
