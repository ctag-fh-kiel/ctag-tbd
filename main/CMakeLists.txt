file(GLOB SRC_FILES *.cpp *.hpp)

# if not CONFIG_TBD_PLATFORM_BBA remove Midi.* files from SRC_FILES
if(NOT CONFIG_TBD_PLATFORM_BBA)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Midi.cpp)
    list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Midi.hpp)
endif()

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES)

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
        )

