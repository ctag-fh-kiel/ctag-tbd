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
set(Boost_USE_STATIC_LIBS ON)
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
set(PLUGIN_SOURCES
        src/plugin.cpp src/WebServer.cpp src/tbd4vcv.cpp src/SPManager.cpp src/SPManagerDataModel.cpp
        "../simulator/fake-idf/esp_heap_caps.c"
        "../simulator/fake-idf/esp_spi_flash.c"
        )

add_library(${LIB_NAME} SHARED ${PLUGIN_SOURCES})
target_include_directories(${LIB_NAME} PUBLIC "${RACK_DIR}/include" "${RACK_DIR}/dep/include")
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
target_link_libraries(${LIB_NAME} PUBLIC ${Boost_LIBRARIES} ctagsp mutable esp-dsp)
if(WIN32)
    #target_link_libraries(tbd-sim -static simple-web-server)
    target_link_libraries(${LIB_NAME} PUBLIC ws2_32 wsock32)
endif()
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

# tbd part
add_compile_definitions(
        RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=16384 # rapidjson
        RAPIDJSON_HAS_STDSTRING=1 # rapidjson
        TBD_SIM=1
        TEST=1
        CONFIG_DSP_ANSI=1
        N_CVS=4
        N_TRIGS=2
        #SPIFFS_PATH="./plugins/tbd4vcv/spiffs_image/" # TODO this has to go in the firmware cmakelists.txt as well + simulator
)
target_compile_options(${LIB_NAME} PRIVATE -Wno-unused-local-typedefs)
set(TBD_SIM 1)
#ctagSoundProcessor
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/rapidjson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/iir/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/add/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/addc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/mul/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/mulc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/mutable/eurorack)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/moog/MoogLadders/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../simulator/fake-idf)

#ctag sound processors
set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor ${CMAKE_CURRENT_BINARY_DIR}/ctagsp)
target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor)
target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen_include)
#target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../main)

#mutable
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../components/mutable ${CMAKE_CURRENT_BINARY_DIR}/mutable)

#rapidjson
file(GLOB RAPIDJSON_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../components/rapidjson/rapidjson/*.h
        )
#esp-dsp
set(ESP_DSP_DIR "../components/esp-dsp")
set(ESP_DSP_SRC_FILES  "${ESP_DSP_DIR}/modules/common/misc/dsps_pwroftwo.cpp"
        "${ESP_DSP_DIR}/modules/dotprod/float/dsps_dotprod_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/dotprod/float/dsps_dotprode_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/dotprod/fixed/dsps_dotprod_s16_ansi.c"
        "${ESP_DSP_DIR}/modules/matrix/float/dspm_mult_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/matrix/fixed/dspm_mult_s16_ansi.c"
        "${ESP_DSP_DIR}/modules/math/mulc/float/dsps_mulc_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/addc/float/dsps_addc_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/add/float/dsps_add_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/sub/float/dsps_sub_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/math/mul/float/dsps_mul_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_d_gen.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_h_gen.c"
        "${ESP_DSP_DIR}/modules/support/misc/dsps_tone_gen.c"
        "${ESP_DSP_DIR}/modules/support/view/dsps_view.cpp"
        "${ESP_DSP_DIR}/modules/windows/hann/float/dsps_wind_hann_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman/float/dsps_wind_blackman_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman_harris/float/dsps_wind_blackman_harris_f32.c"
        "${ESP_DSP_DIR}/modules/windows/blackman_nuttall/float/dsps_wind_blackman_nuttall_f32.c"
        "${ESP_DSP_DIR}/modules/windows/nuttall/float/dsps_wind_nuttall_f32.c"
        "${ESP_DSP_DIR}/modules/windows/flat_top/float/dsps_wind_flat_top_f32.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_conv_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_corr_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/conv/float/dsps_ccorr_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/iir/biquad/dsps_biquad_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/iir/biquad/dsps_biquad_gen_f32.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fir_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fir_init_f32.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fird_f32_ansi.c"
        "${ESP_DSP_DIR}/modules/fir/float/dsps_fird_init_f32.c")

set(ESP_DSP_INC_DIRS   "${ESP_DSP_DIR}/modules/dotprod/include"
        "${ESP_DSP_DIR}/modules/support/include"
        "${ESP_DSP_DIR}/modules/windows/include"
        "${ESP_DSP_DIR}/modules/windows/hann/include"
        "${ESP_DSP_DIR}/modules/windows/blackman/include"
        "${ESP_DSP_DIR}/modules/windows/blackman_harris/include"
        "${ESP_DSP_DIR}/modules/windows/blackman_nuttall/include"
        "${ESP_DSP_DIR}/modules/windows/nuttall/include"
        "${ESP_DSP_DIR}/modules/windows/flat_top/include"
        "${ESP_DSP_DIR}/modules/iir/include"
        "${ESP_DSP_DIR}/modules/fir/include"
        "${ESP_DSP_DIR}/modules/math/include"
        "${ESP_DSP_DIR}/modules/math/add/include"
        "${ESP_DSP_DIR}/modules/math/sub/include"
        "${ESP_DSP_DIR}/modules/math/mul/include"
        "${ESP_DSP_DIR}/modules/math/addc/include"
        "${ESP_DSP_DIR}/modules/math/mulc/include"
        "${ESP_DSP_DIR}/modules/matrix/include"
        "${ESP_DSP_DIR}/modules/fft/include"
        "${ESP_DSP_DIR}/modules/dct/include"
        "${ESP_DSP_DIR}/modules/conv/include"
        "${ESP_DSP_DIR}/modules/common/include")

add_library(esp-dsp STATIC ${ESP_DSP_SRC_FILES})
target_include_directories(esp-dsp PRIVATE ${ESP_DSP_INC_DIRS})


# install
#install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
#install(FILES plugin.json DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
#install(FILES ../sample_rom/sample-rom.tbd DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME}/sample-rom)
#install(DIRECTORY res DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../spiffs_image DESTINATION ${RACK_DIR}/plugins/${PLUGIN_NAME})
#install(DIRECTORY ${CMAKE_BINARY_DIR}/www DESTINATION .)


install(TARGETS ${LIB_NAME} LIBRARY DESTINATION tbd4vcv)
install(FILES plugin.json DESTINATION tbd4vcv)
install(FILES ../sample_rom/sample-rom.tbd DESTINATION tbd4vcv/sample-rom)
install(DIRECTORY res DESTINATION tbd4vcv)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../spiffs_image DESTINATION tbd4vcv)
install(FILES ../LICENSE readme.md DESTINATION tbd4vcv)
#install(FILES ${Boost_LIBRARY_DIRS}/libboost_system-mt.dylib DESTINATION tbd4vcv)

# package generation
set(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "tbd4vcv")
SET(CPACK_PACKAGE_VENDOR "ctag-tbd")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
# Tell CPack not to put everything inside an enclosing directory.
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
# Apparently this should be always on but isn't for backwards compatibility.
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)