# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    include/tbd/**/*.hpp    
    src/*.cpp
    src/*.hpp
)

add_library(tbd_sound_manager STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_manager
    PUBLIC include
)
target_link_libraries(tbd_sound_manager 
    PUBLIC tbd_sound_registry
    PRIVATE 
        tbd_system 
        tbd_drivers
        tbd_presets
        tbd_config
)

target_compile_options(tbd_sound_manager PRIVATE
        -Wno-unused-local-typedefs
        -ffast-math
)
