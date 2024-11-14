set(PORTS_COMMON_DIR ${CMAKE_SOURCE_DIR}/ports/common)
set(PORTS_SYSTEM_COMMON_INCLUDE ${PORTS_COMMON_DIR}/tbd_system/include)

file(GLOB SRCS_FILES 
    include/tbd/*.hpp
    ${PORTS_SYSTEM_COMMON_INCLUDE}/tbd/*.hpp
    src/*.hpp
    src/*.cpp
)

add_library(tbd_system STATIC ${SRCS_FILES})
target_include_directories(tbd_sounds PRIVATE src) 
target_link_libraries(tbd_sounds
    PUBLIC tbd_sound_processor tbd_helpers moog mutable
    PRIVATE __idf_tbd_system __idf_tbd_system mutable __idf_espressif__esp-dsp
)
