file(GLOB SRCS_FILES 
        include/tbd/**/*.hpp
        src/*.hpp
        src/*.cpp
)

add_library(tbd_config STATIC ${SRCS_FILES})
target_include_directories(tbd_config PUBLIC include)
target_link_libraries(tbd_config PUBLIC rapidjson)
