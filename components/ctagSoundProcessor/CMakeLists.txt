include(${COMPONENT_DIR}/cmake/bin2h.cmake)
include(${COMPONENT_DIR}/cmake/JSONParser.cmake)

# get all sound processor files
file(GLOB SOUND_PROCESSORS ctagSoundProcessor*.cpp)
# clean generated directory
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/gen_include)
# convert JSON descriptors to c headers and extract processor IDs
set(SP_INCLUDES "" PARENT_SCOPE)
set(BIG_IF "" PARENT_SCOPE)
foreach(VAR ${SOUND_PROCESSORS})
    # check if JSON file for sound processor exists
    get_filename_component(MYFILE_WITHOUT_EXT ${VAR} NAME_WLE)
    string(REGEX REPLACE "ctagSoundProcessor+" "" SP_ID ${MYFILE_WITHOUT_EXT} )
    string(TOLOWER ${SP_ID} SP_ID)
    set(SP_INCLUDES "${SP_INCLUDES}#include \"${MYFILE_WITHOUT_EXT}.hpp\"\n")
    # prepare big if variable for factory
    set(BIG_IF "${BIG_IF}if(type.compare(\"${SP_ID}\") == 0) return std::make_unique<${MYFILE_WITHOUT_EXT}>();\n")
endforeach()

# write sound processor descriptor json and convert it to header file
file(WRITE ${CMAKE_BINARY_DIR}/gen_include/ctagSoundProcessors.hpp ${SP_INCLUDES})

# create big if structure for factory
configure_file(${COMPONENT_DIR}/ctagSoundProcessorFactory.hpp.in ${CMAKE_BINARY_DIR}/gen_include/ctagSoundProcessorFactory.hpp @ONLY )

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
    ${CMAKE_BINARY_DIR}/gen_include/*.hpp
)


idf_component_register(SRCS ${SRCS_FILES} 
                    INCLUDE_DIRS . ${CMAKE_BINARY_DIR}/gen_include
                    PRIV_REQUIRES rapidjson esp-dsp)

#set_source_files_properties(ctagSoundProcessorGDVerb.cpp
#        PROPERTIES COMPILE_FLAGS
#        -Wno-strict-aliasing
#        )
