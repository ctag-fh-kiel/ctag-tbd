file(GLOB SRCS_FILES 
    include/**/*.hpp
    src/*.hpp
    src/*.cpp
)

add_library(tbd_sound_manager_port STATIC ${SRCS_FILES})
target_include_directories(tbd_sound_manager_port
        PUBLIC
            include
            "${CMAKE_SOURCE_DIR}/vendor/rtaudio"
)
target_link_libraries(tbd_sound_manager_port 
        PUBLIC
            tbd_system
            tbd_sound_manager_common
            tbd_common
            tbd_portutils
        PRIVATE
            rtaudio
)
