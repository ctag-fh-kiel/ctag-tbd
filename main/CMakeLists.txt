file(GLOB SRC_FILES *.cpp *.hpp)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES)

target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions -Wno-multichar -Wno-strict-aliasing)
# number of CV ins
if(CONFIG_TBD_PLATFORM_STR)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE N_CVS=8)
else()
    target_compile_definitions(${COMPONENT_LIB} PRIVATE N_CVS=4)
endif()

