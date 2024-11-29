file(GLOB SRCS_FILES 
        include/**/*.hpp
        src/*.hpp
        src/*.cpp
        ${CMAKE_SOURCE_DIR}/vendor/tinywav/tinywav.h
        ${CMAKE_SOURCE_DIR}/vendor/tinywav/tinywav.c
)

add_library(tbd_portutils STATIC ${SRCS_FILES})
target_include_directories(tbd_portutils
        PUBLIC
            include
            "${CMAKE_SOURCE_DIR}/vendor/tinywav"
)
