# get all sound processor files
file(GLOB SOUND_PROCESSORS ctagSoundProcessor*.cpp)
# clean generated directory
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/gen_include)
# convert JSON descriptors to c headers and extract processor IDs
set(SP_INCLUDES "" PARENT_SCOPE)
set(BIG_IF "" PARENT_SCOPE)
foreach (VAR ${SOUND_PROCESSORS})
    # check if JSON file for sound processor exists
    # get_filename_component(MYFILE_WITHOUT_EXT ${VAR} NAME_WLE) # needs newer CMake Version > 3.13 which is not in IDF r4.1 docker image
    get_filename_component(MYFILE_WITHOUT_EXT ${VAR} NAME_WE)
    string(REGEX REPLACE "\\.[^.]*$" "" MYFILE_WITHOUT_EXT ${MYFILE_WITHOUT_EXT}) # using regex instead of NAME_WLE
    string(REGEX REPLACE "ctagSoundProcessor+" "" SP_ID ${MYFILE_WITHOUT_EXT})
    set(SP_INCLUDES "${SP_INCLUDES}#include \"${MYFILE_WITHOUT_EXT}.hpp\"\n")
    # prepare big if variable for factory
    set(BIG_IF "${BIG_IF}if(type.compare(\"${SP_ID}\") == 0) return std::make_unique<${MYFILE_WITHOUT_EXT}>();\n")
endforeach ()

# write sound processor descriptor json and convert it to header file
file(WRITE ${CMAKE_BINARY_DIR}/gen_include/ctagSoundProcessors.hpp ${SP_INCLUDES})

# create big if structure for factory
configure_file(${COMPONENT_DIR}/ctagSoundProcessorFactory.hpp.in ${CMAKE_BINARY_DIR}/gen_include/ctagSoundProcessorFactory.hpp @ONLY)

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
        tesselode/*.h
        tesselode/*.cpp
        airwindows/*.hpp
        airwindows/*.cpp
        synthesis/*.hpp
        synthesis/*.cpp
        vult/*.cpp
        vult/*.h
        ${CMAKE_BINARY_DIR}/gen_include/*.hpp
        )

if (TBD_SIM)
    add_library(ctagsp STATIC ${SRCS_FILES})
    target_include_directories(ctagsp PRIVATE . ${CMAKE_BINARY_DIR}/gen_include)
    target_compile_definitions(ctagsp PUBLIC N_CVS=4 N_TRGS=2)
else ()
    idf_component_register(SRCS ${SRCS_FILES}
            INCLUDE_DIRS . ${CMAKE_BINARY_DIR}/gen_include
            PRIV_REQUIRES rapidjson esp-dsp mutable moog spi_flash)
    # number of CV ins
    if(CONFIG_TBD_PLATFORM_STR)
        target_compile_definitions(${COMPONENT_LIB} PUBLIC N_CVS=8 N_TRGS=2)
    else()
        target_compile_definitions(${COMPONENT_LIB} PUBLIC N_CVS=4 N_TRGS=2)
    endif()
endif ()


