# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    include/tbd/**/*.hpp    
    src/*.cpp
    src/*.hpp
)

tbd_platform_display("${TBD_PLATFORM_OBJ}" VAR has_display)
if (has_display)
    set(display_lib tbd_display)
endif()


add_library(tbd_presets STATIC ${SRCS_FILES})

target_include_directories(tbd_presets
    PUBLIC include
)
target_link_libraries(tbd_presets
    PUBLIC tbd_sound_manager
    PRIVATE 
        ${display_lib}
        tbd_drivers
)

target_compile_options(tbd_presets PRIVATE
        -Wno-unused-local-typedefs
)
