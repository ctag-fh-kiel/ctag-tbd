# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(tbd-sim)

add_definitions(
        -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 # rapidjson
        -DRAPIDJSON_HAS_STDSTRING=1 # rapidjson
        -DTBD_SIM=1
        -Wno-unused-local-typedefs
        -DTEST
        -DCONFIG_DSP_ANSI=1
)

set(TBD_SIM 1)

## Web Server Part
add_library(simple-web-server INTERFACE)
target_include_directories(simple-web-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Simple-Web-Server)
find_package(Threads REQUIRED)
target_link_libraries(simple-web-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})
find_package(Boost 1.53.0 COMPONENTS system thread REQUIRED)
target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    target_compile_definitions(simple-web-server INTERFACE USE_BOOST_REGEX)
    find_package(Boost 1.53.0 COMPONENTS regex REQUIRED)
    target_link_libraries(simple-web-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-web-server INTERFACE ${Boost_INCLUDE_DIR})
endif()

#rtaudio
set(RTAUDIO_BUILD_STATIC_LIBS TRUE)
add_subdirectory(rtaudio)

#ctagSoundProcessor
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/rapidjson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/iir/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/add/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/addc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/mul/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/math/mulc/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/esp-dsp/modules/common/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../components/mutable/eurorack)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fake-idf)

set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor ${CMAKE_CURRENT_BINARY_DIR}/ctagsp)

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

## Application
find_package(Boost 1.53.0 COMPONENTS system thread filesystem program_options)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "could not find Boost!")
endif()

file(GLOB SRC_FILES *.cpp *.hpp)
set(SRC_FILES2
        "../main/SPManagerDataModel.cpp"
        "fake-idf/esp_heap_caps.c"
        )

add_executable(tbd-sim ${SRC_FILES} ${SRC_FILES2} ${RAPIDJSON_FILES})
target_link_libraries(tbd-sim simple-web-server)
target_link_libraries(tbd-sim rtaudio ctagsp mutable esp-dsp)
target_link_libraries(tbd-sim ${Boost_LIBRARIES})
target_include_directories(tbd-sim PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(tbd-sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../components/ctagSoundProcessor)
target_include_directories(tbd-sim PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen_include)
target_include_directories(tbd-sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rtaudio)
target_include_directories(tbd-sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../main)




