# add sources to config, headers added for intellij
file(GLOB SRCS_FILES
        rapidjson/*.h
)

add_library(rapidjson STATIC ${SRCS_FILES})
target_include_directories(rapidjson PUBLIC .)
target_link_libraries(rapidjson PUBLIC __idf_heap)
target_compile_definitions(rapidjson 
        PUBLIC RAPIDJSON_HAS_STDSTRING=1 
        DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096        
)
set_target_properties(rapidjson PROPERTIES LINKER_LANGUAGE CXX)
