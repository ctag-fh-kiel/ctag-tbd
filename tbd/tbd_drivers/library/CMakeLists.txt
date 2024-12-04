tbd_platform_audio("${TBD_PLATFORM_OBJ}")
tbd_codec_type("${_return}")
if ("${_return}" STREQUAL "sim_codec")
    list(APPEND extra_srcs src/sim_codec.cpp)
endif()

tbd_platform_cv_input("${TBD_PLATFORM_OBJ}")
tbd_cv_input_type("${_return}")
if ("${_return}" STREQUAL "sim_cv")
    list(APPEND extra_srcs src/sim_cv.cpp)
endif()

tbd_platform_indicator("${TBD_PLATFORM_OBJ}")
tbd_indicator_type("${_return}")
if ("${_return}" STREQUAL "sim_indicator")
    list(APPEND extra_srcs src/sim_indicator.cpp)
endif()

file(GLOB SRCS_FILES
        include/tbd/drivers/*.hpp
)

add_library(tbd_drivers STATIC ${SRCS_FILES} ${extra_srcs})
target_include_directories(tbd_drivers PUBLIC include)
target_link_libraries(tbd_drivers PUBLIC ${TBD_DRIVERS_PORT_LIB} tbd_common)
