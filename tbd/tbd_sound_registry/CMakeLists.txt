set(HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/tbd/sound_registry)
set(REGISTRY_HEADER ${HEADER_DIR}/sound_processor_factory.hpp)
file(GLOB SOUND_PROCESSORS ${CMAKE_SOURCE_DIR}/components/tbd_sounds/src/ctagSoundProcessor*.cpp)

set(SRCS_FILES
    ${REGISTRY_HEADER}
)

add_custom_command(
    OUTPUT ${REGISTRY_HEADER} 
    COMMAND ${TBD_TOOL} plugins create-factory -o "${REGISTRY_HEADER}"
    DEPENDS ${SOUND_PROCESSORS}   
)

add_library(tbd_sound_registry INTERFACE ${SRCS_FILES})
target_include_directories(tbd_sound_registry INTERFACE include)
target_link_libraries(tbd_sound_registry
        INTERFACE tbd_sound_processor tbd_sounds
)
