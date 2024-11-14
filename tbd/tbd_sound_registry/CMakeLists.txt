set(HEADER_DIR ${COMPONENT_DIR}/include/tbd/sound_registry)
set(REGISTRY_HEADER ${HEADER_DIR}/ctagSoundProcessorFactory.hpp)
file(GLOB SOUND_PROCESSORS ${CMAKE_SOURCE_DIR}/components/tbd_sounds/src/ctagSoundProcessor*.cpp)

set(SRCS_FILES
    ${REGISTRY_HEADER}
)

add_custom_command(
    OUTPUT ${REGISTRY_HEADER} 
    COMMAND ${TBD_TOOL} plugins create-factory
    DEPENDS ${SOUND_PROCESSORS}   
)

add_library(tbd_sound_registry STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_registry PUBLIC include)
target_link_libraries(tbd_sound_registry 
    PUBLIC tbd_sound_processor tbd_sounds
)
