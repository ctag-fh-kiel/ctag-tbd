# get all sound processor files
file(GLOB SOUND_PROCESSORS ctagSoundProcessor*.cpp)
# remove item matching "ctagSoundProcessor.cpp" from ${SOUND_PROCESSORS}
# list(FILTER SOUND_PROCESSORS EXCLUDE REGEX "ctagSoundProcessor\\.cpp$")
# clean generated directory
set(SP_GEN_INCLUDE_DIR "${CMAKE_BINARY_DIR}/gen_include")
file(REMOVE_RECURSE "${SP_GEN_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${SP_GEN_INCLUDE_DIR}")
# convert JSON descriptors to c headers and extract processor IDs
set(SP_INCLUDES "")
set(BIG_IF "")
foreach (VAR ${SOUND_PROCESSORS})
    # check if JSON file for sound processor exists
    get_filename_component(MYFILE_WITHOUT_EXT ${VAR} NAME_WE)
    message(STATUS "Processing ${MYFILE_WITHOUT_EXT}")
    string(REGEX REPLACE "\\.[^.]*$" "" MYFILE_WITHOUT_EXT ${MYFILE_WITHOUT_EXT}) # using regex instead of NAME_WLE
    string(REGEX REPLACE "ctagSoundProcessor+" "" SP_ID ${MYFILE_WITHOUT_EXT})
    set(SP_INCLUDES "${SP_INCLUDES}#include \"${MYFILE_WITHOUT_EXT}.hpp\"\n")
    # prepare big if variable for factory
    set(BIG_IF "${BIG_IF}if(type.compare(\"${SP_ID}\") == 0) processor = new ${MYFILE_WITHOUT_EXT}();\n")
endforeach ()

# write sound processor descriptor json and convert it to header file
file(WRITE "${SP_GEN_INCLUDE_DIR}/ctagSoundProcessors.hpp" "${SP_INCLUDES}")

# create big if structure for factory
configure_file("${CMAKE_CURRENT_LIST_DIR}/ctagSoundProcessorFactory.hpp.in" "${SP_GEN_INCLUDE_DIR}/ctagSoundProcessorFactory.hpp" @ONLY)

# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
        *.cpp
        *.hpp
        helpers/*.cpp
        helpers/*.hpp
        gverb/*.c
        gverb/*.h
        freeverb/*.cc
        freeverb/*.hh
        freeverb/*.hpp
        freeverb3/*.hpp
        freeverb3/*.cpp
        freeverb3/*.h
        mifx/*.h
        polypad/*.hpp
        polypad/*.cpp
        filters/*.hpp
        filters/*.cpp
        fx/*.hpp
        fx/*.cpp
        tesselode/*.h
        tesselode/*.cpp
        airwindows/*.hpp
        airwindows/*.cpp
        synthesis/*.hpp
        synthesis/*.cpp
        vult/*.cpp
        vult/*.h
        memory/*.h
        memory/*.c
        SimpleComp/*.h
        SimpleComp/*.cpp
        "${SP_GEN_INCLUDE_DIR}/*.hpp"
        )

if (TBD_SIM)
    add_library(ctagsp STATIC ${SRCS_FILES})
    target_include_directories(ctagsp PRIVATE . "${SP_GEN_INCLUDE_DIR}")
    target_compile_definitions(ctagsp PRIVATE CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA=29360128)
    target_compile_definitions(ctagsp PRIVATE CONFIG_SP_FIXED_MEM_ALLOC_SZ=307200)
    target_compile_definitions(ctagsp PRIVATE CONFIG_MAX_SAMPLES_IN_SAMPLE_ROM=1024)
else ()
    idf_component_register(SRCS ${SRCS_FILES}
            INCLUDE_DIRS . "${SP_GEN_INCLUDE_DIR}"
            REQUIRES mutable esp-dsp moog
            PRIV_REQUIRES rapidjson spi_flash)
    target_compile_options(${COMPONENT_LIB} PRIVATE
            -Wno-unused-local-typedefs
            -ffast-math
            )
endif ()
