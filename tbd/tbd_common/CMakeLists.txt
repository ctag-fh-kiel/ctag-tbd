file(GLOB SRCS_FILES 
        include/**/*.hpp
        src/*.hpp
        src/*.cpp
)

add_library(tbd_common INTERFACE ${SRCS_FILES})
target_include_directories(tbd_common INTERFACE include)
