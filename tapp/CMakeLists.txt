cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tapp)

add_definitions(
        -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 # rapidjson
        -DRAPIDJSON_HAS_STDSTRING=1 # rapidjson
)

# everything static linking
#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_EXE_LINKER_FLAGS "-static")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# link boost statically
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.67.0 COMPONENTS system thread filesystem program_options REQUIRED)

## Web Server Part
add_library(simple-web-server INTERFACE)
target_include_directories(simple-web-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../simulator/Simple-Web-Server)
find_package(Threads REQUIRED)
target_link_libraries(simple-web-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    target_compile_definitions(simple-web-server INTERFACE USE_BOOST_REGEX)
    find_package(Boost 1.67.0 COMPONENTS regex REQUIRED)
    target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
endif()


#rapidjson
file(GLOB RAPIDJSON_FILES
        ${CMAKE_SOURCE_DIR}/../components/rapidjson/rapidjson/*.h
)

#tapp
file(GLOB SRC_FILES *.cpp *.hpp)
add_executable(tapp ${SRC_FILES} ${CMAKE_SOURCE_DIR}/../simulator/fake-idf/esp_heap_caps.c)
#target_link_options(tapp PRIVATE -static-libgcc -static-libstdc++)

if(WIN32)
    target_link_libraries(tapp -static simple-web-server)
    target_link_libraries(tapp -static ${Boost_LIBRARIES})
    target_link_libraries(tapp -static ws2_32 wsock32)
elseif(APPLE)
    target_link_libraries(tapp simple-web-server)
    target_link_libraries(tapp ${Boost_LIBRARIES})
else() #linux etc?!
    target_link_libraries(tapp simple-web-server)
    target_link_libraries(tapp ${Boost_LIBRARIES})
endif()

target_include_directories(tapp PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(tapp PRIVATE ${CMAKE_SOURCE_DIR}/../components/rapidjson)
target_include_directories(tapp PRIVATE ${CMAKE_SOURCE_DIR}/../simulator/fake-idf)
add_dependencies(tapp copy-www)

# copy spiffs files to build folder
add_custom_target(copy-www
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/../spiffs_image/www
        ${CMAKE_BINARY_DIR}/www
        )

install(CODE "set(CMAKE_INSTALL_LOCAL_ONLY true)")
install(TARGETS tapp RUNTIME DESTINATION .)
install(DIRECTORY ${CMAKE_BINARY_DIR}/www DESTINATION .)

# package generation
set(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tbd-sim")
SET(CPACK_PACKAGE_VENDOR "ctag-tbd")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
#SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
include(CPack)