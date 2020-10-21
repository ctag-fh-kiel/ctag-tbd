# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
        -DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096 # rapidjson
        -DRAPIDJSON_HAS_STDSTRING=1 # rapidjson
        -Wno-unused-local-typedefs
        # https://stackoverflow.com/questions/22931147/stdisinf-does-not-work-with-ffast-math-how-to-check-for-infinity
        # -ffast-math
        # -ffast-math -fno-finite-math-only
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ctag-tbd)

