#pragma once

#include <cstdlib>
#include <cinttypes>
#include <tbd/errors.hpp>

TBD_NEW_ERR(API_BAD_ENDPOINT, "failed to parse endpoint header");
TBD_NEW_ERR(API_BUFFER_SIZE, "message size exceeds max buffer size");
TBD_NEW_ERR(API_DECODE, "failed to deserialize message");
TBD_NEW_ERR(API_ENCODE, "failed to serialize message");

namespace tbd {

struct Api {
    Api() = delete;

    using EndpointCallback = uint32_t(*)(uint8_t*, size_t&);
    static uint32_t handle_stream_input(uint8_t* buffer, size_t& length);

};

}
