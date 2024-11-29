file(GLOB SRCS_FILES 
    include/**/*.hpp
    src/*.hpp
    src/*.cpp
)

idf_component_register(
    SRCS ${SRCS_FILES}
    INCLUDE_DIRS include
)

target_link_libraries(${COMPONENT_LIB}
        PUBLIC
            tbd_common
            tbd_sound_manager_common
            tbd_drivers
)
