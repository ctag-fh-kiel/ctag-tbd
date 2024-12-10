tbd_platform_file_system("${TBD_PLATFORM_OBJ}" VAR file_system)

if ("${file_system}" STREQUAL "std" AND "${TBD_TOOLCHAIN}" STREQUAL "esp32")
    tbd_loge("esp32 platform does not support std::filesystem at this point, use 'wrapper'")
endif()

if ("${file_system}" STREQUAL "wrapper")
    set(extra_srcs src/filesystem_wrapper.cpp)
    set(defines TBD_FILE_SYSTEM_USE_WRAPPER=1)
else()
    set(defines TBD_FILE_SYSTEM_USE_WRAPPER=0)
endif()

file(GLOB SRCS_FILES
        include/tbd/storage/*.hpp
        src/dummy.cpp
        ${extra_srcs}
)

add_library(tbd_storage_common STATIC ${SRCS_FILES})
target_compile_definitions(tbd_storage_common PUBLIC ${defines})
target_include_directories(tbd_storage_common PUBLIC include)
target_link_libraries(tbd_storage_common PUBLIC tbd_system)

message("
TBD storage lib configuration
-----------------------------
file system: ${file_system}
-----------------------------
")
