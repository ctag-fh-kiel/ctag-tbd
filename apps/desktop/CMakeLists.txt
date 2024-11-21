
file(GLOB SRCS_FILES
    src/*.hpp
    src/*.cpp
)

add_executable(tbd_app_desktop ${SRCS_FILES})
