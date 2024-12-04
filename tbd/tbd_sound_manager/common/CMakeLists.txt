tbd_platform_audio("${TBD_PLATFORM_OBJ}")
tbd_codec_work_type_flags("${_return}" VAR work_type_flags)

file(GLOB SRCS_FILES
        include/tbd/**/*.hpp
)

add_library(tbd_sound_manager_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_sound_manager_common INTERFACE include)
target_compile_definitions(tbd_sound_manager_common INTERFACE ${work_type_flags})

