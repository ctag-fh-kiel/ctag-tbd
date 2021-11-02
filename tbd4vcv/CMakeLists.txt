cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RACK_DIR $ENV{RACK_DIR})

if ("${RACK_DIR}" STREQUAL "")
    message(FATAL_ERROR "Path to Rack SDK missing! Add -DRACK_DIR=<path to Rack SDK> to the cmake call.")
endif ()

message(STATUS "Use Rack SDK: ${RACK_DIR}")

# set this to the plugin slug!
set(PLUGIN_NAME tbd4vcv)

project(${PLUGIN_NAME})

## Web Server Part
add_library(simple-web-server INTERFACE)
target_include_directories(simple-web-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Simple-Web-Server)
find_package(Threads REQUIRED)
target_link_libraries(simple-web-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})
find_package(Boost 1.67.0 REQUIRED COMPONENTS system filesystem thread )
target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    target_compile_definitions(simple-web-server INTERFACE USE_BOOST_REGEX)
    find_package(Boost 1.67.0 COMPONENTS regex REQUIRED)
    target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
endif()
if(NOT TARGET Boost::filesystem)
    add_library(Boost::filesystem IMPORTED INTERFACE)
    set_property(TARGET Boost::filesystem PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set_property(TARGET Boost::filesystem PROPERTY
            INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
endif()

# Plugin part
set(LIB_NAME plugin)
#file(GLOB_RECURSE PLUGIN_SOURCES src/*.cpp uWebSockets/uSockets/src/*.c)
set(PLUGIN_SOURCES src/plugin.cpp src/WebServer.cpp src/tbd4vcv.cpp)


add_library(${LIB_NAME} SHARED ${PLUGIN_SOURCES})
target_include_directories(${LIB_NAME} PUBLIC "${RACK_DIR}/include" "${RACK_DIR}/dep/include")
target_include_directories(${LIB_NAME} PUBLIC "uWebSockets/src/" "uWebSockets/uSockets/src")
target_include_directories(${LIB_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../simulator/Simple-Web-Server")
target_include_directories(${LIB_NAME} PUBLIC ${Boost_INCLUDE_DIR})

if(WIN32)
    target_compile_definitions(${LIB_NAME} PRIVATE -DARCH_WIN)
    #target_link_libraries(${LIB_NAME} "Rack")
elseif(APPLE)
    target_compile_definitions(${LIB_NAME} PRIVATE -DARCH_MAC)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
    endif()
    #message(FATAL_ERROR ${CMAKE_SHARED_LINKER_FLAGS})
else()
    target_compile_definitions(${LIB_NAME} PRIVATE -DARCH_LIN)
endif()
target_compile_options(${LIB_NAME} PRIVATE -fPIC)
target_compile_definitions(${LIB_NAME} PUBLIC LIBUS_NO_SSL UWS_NO_ZLIB)
target_link_libraries(${LIB_NAME} PUBLIC ${Boost_LIBRARIES})
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

# install
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
install(FILES plugin.json DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
install(DIRECTORY res DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../spiffs_image DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
#install(DIRECTORY ${CMAKE_BINARY_DIR}/www DESTINATION .)

# package generation
set(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tbd4vcv")
SET(CPACK_PACKAGE_VENDOR "ctag-tbd")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
#SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
include(CPack)