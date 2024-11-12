# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
    include/tbd/helpers/*.hpp
    src/*.hpp
    src/*.cpp
)

add_library(tbd_helpers STATIC ${SRCS_FILES})
target_include_directories(tbd_helpers PUBLIC include)
target_link_libraries(tbd_helpers 
    PRIVATE __idf_heap __idf_espressif__esp-dsp __idf_nvs_flash
)
target_compile_options(tbd_helpers PRIVATE 
        -Wno-unused-local-typedefs
        -ffast-math
)


