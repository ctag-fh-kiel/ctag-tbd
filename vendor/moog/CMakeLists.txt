# add sources to config, headers added for intellij
set(SRCS_FILES
        MoogLadders/src/Filters.h
        MoogLadders/src/HuovilainenModel.h
        MoogLadders/src/ImprovedModel.h
        MoogLadders/src/KrajeskiModel.h
        MoogLadders/src/LadderFilterBase.h
        MoogLadders/src/MicrotrackerModel.h
        MoogLadders/src/MusicDSPModel.h
        MoogLadders/src/NoiseGenerator.h
        MoogLadders/src/OberheimVariationModel.h
        MoogLadders/src/RKSimulationModel.h
        MoogLadders/src/RingBuffer.h
        MoogLadders/src/SimplifiedModel.h
        MoogLadders/src/StilsonModel.h
        MoogLadders/src/util.h
        )

add_library(moog STATIC ${SRCS_FILES})
target_include_directories(moog PUBLIC "MoogLadders/src")
set_target_properties(moog PROPERTIES LINKER_LANGUAGE CXX)
