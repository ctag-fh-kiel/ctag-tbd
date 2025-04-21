file(GLOB SRCS_FILES
        include/**/*.hpp
        src/*.cpp
        src/*.hpp
)

add_library(tbd_calibration STATIC ${SRCS_FILES})
target_include_directories(tbd_calibration
        PUBLIC include
)
target_link_libraries(tbd_calibration
        PRIVATE
        tbd_system
        tbd_config
        tbd_drivers
)
target_compile_options(tbd_calibration PRIVATE
        -Wno-unused-local-typedefs
)
