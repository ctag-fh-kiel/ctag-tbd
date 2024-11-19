# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    src/*.cpp
    src/*.hpp
    include/tbd/**/*.hpp
    utils/gverb/*.c
    utils/gverb/*.h
    utils/freeverb/*.cc
    utils/freeverb/*.hh
    utils/freeverb/*.hpp
    utils/freeverb3/*.hpp
    utils/freeverb3/*.cpp
    utils/freeverb3/*.h
    utils/mifx/*.h
    utils/polypad/*.hpp
    utils/polypad/*.cpp
    utils/filters/*.hpp
    utils/filters/*.cpp
    utils/fx/*.hpp
    utils/fx/*.cpp
    utils/tesselode/*.h
    utils/tesselode/*.cpp
    utils/airwindows/*.hpp
    utils/airwindows/*.cpp
    utils/synthesis/*.hpp
    utils/synthesis/*.cpp
    utils/vult/*.cpp
    utils/vult/*.h
    utils/memory/*.h
    utils/memory/*.c
    utils/SimpleComp/*.h
    utils/SimpleComp/*.cpp
)

add_library(tbd_sound_processor STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_processor 
    PUBLIC include utils
)
target_link_libraries(tbd_sound_processor 
    PUBLIC tbd_config
    PRIVATE 
        tbd_helpers
        mutable
        tbd_system
        __idf_espressif__esp-dsp 
)
target_compile_options(tbd_sound_processor PRIVATE
        -Wno-unused-local-typedefs
        -ffast-math
)
