find_package(Boost 1.67.0 COMPONENTS system thread REQUIRED)

file(GLOB SRCS_FILES
    include/tbd/**/*.hpp
    src/dummy.cpp
)

add_library(tbd_system_port STATIC ${SRCS_FILES})
target_include_directories(tbd_system_port
        PUBLIC include
        INTERFACE ${Boost_INCLUDE_DIR}
)
target_link_libraries(tbd_system_port
        PUBLIC tbd_system_common
        INTERFACE ${Boost_LIBRARIES}
)
