file(GLOB SRCS_FILES 
    src/*.cpp
    src/*.hpp
    "include/tbd/sounds/*.hpp"
)

add_library(tbd_sounds STATIC ${SRCS_FILES}) 
target_include_directories(tbd_sounds PUBLIC include) 
target_include_directories(tbd_sounds PRIVATE src) 
target_link_libraries(tbd_sounds
    PUBLIC tbd_sound_processor tbd_helpers moog mutable
    PRIVATE __idf_tbd_system __idf_tbd_system mutable __idf_espressif__esp-dsp
)
