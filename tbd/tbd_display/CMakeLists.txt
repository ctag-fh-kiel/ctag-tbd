# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    include/tbd/**/*.hpp    
    src/*.cpp
    src/*.hpp
)

add_library(tbd_display STATIC ${SRCS_FILES})
target_include_directories(tbd_display
    PUBLIC include
)
target_link_libraries(tbd_display
    PUBLIC
    PRIVATE 
        tbd_version
        tbd_drivers
)

target_compile_options(tbd_display PRIVATE
        -Wno-unused-local-typedefs
        -ffast-math
)
