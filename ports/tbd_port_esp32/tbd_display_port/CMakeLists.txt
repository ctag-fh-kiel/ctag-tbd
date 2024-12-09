include(${PROJECT_DIR}/tools/cmake/build_config/platforms.cmake)

tbd_platform_display("${TBD_PLATFORM_OBJ}" VAR has_display)
if (has_display)

file(GLOB SRCS_FILES
    src/*.cpp
    src/*.hpp
)

idf_component_register(
    SRCS ${SRCS_FILES}
    REQUIRES tbd_drivers_port
)
target_link_libraries(${COMPONENT_LIB} PUBLIC tbd_display_common tbd_version)

endif()