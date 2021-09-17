file(GLOB SRC_FILES *.cpp *.hpp)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES)

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-unused-local-typedefs
        )

