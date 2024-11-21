file(GLOB SRCS_FILES 
        include/tbd/**/*.hpp
)

add_library(tbd_sound_manager_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_sound_manager_common INTERFACE include)

