#pragma once

#include <cstdlib>
#include <cinttypes>

namespace tbd {

namespace errors {
    constexpr uint32_t SUCCESS = 0;
    constexpr uint32_t FAILURE = 1;
    constexpr uint32_t API_BAD_ENDPOINT = 1;
    constexpr uint32_t API_BAD_HEADER = 2;
    constexpr uint32_t API_BUFFER_SIZE = 3;
    constexpr uint32_t API_DECODE = 4;
    constexpr uint32_t API_ENCODE = 5;
}

struct Api {
    Api() = delete;

    using EndpointCallback = uint32_t(*)(uint8_t*, size_t&);
    static uint32_t handle_stream_input(uint8_t* buffer, size_t& length);

};

}
