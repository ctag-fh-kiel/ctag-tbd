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
)

add_library(tbd_sound_processor STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_processor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tbd_sound_processor 
    PUBLIC rapidjson 
    PRIVATE tbd_helpers mutable moog 
            __idf_tbd_system __idf_nvs_flash __idf_esp_system __idf_esp_system __idf_espressif__esp-dsp
)
target_compile_options(tbd_sound_processor PRIVATE
        -Wno-unused-local-typedefs
        -ffast-math
)
