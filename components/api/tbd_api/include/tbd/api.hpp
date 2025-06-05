#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/errors.hpp>

TBD_NEW_ERR(API_WRONG_PACKET_TYPE, "packet type does not match handler");
TBD_NEW_ERR(API_BAD_ENDPOINT, "invalid endpoint ID");
TBD_NEW_ERR(API_RESPONSE_BUFFER_SIZE, "output buffer too small for response payload");
TBD_NEW_ERR(API_DECODE, "failed to deserialize message");
TBD_NEW_ERR(API_ENCODE, "failed to serialize message");
TBD_NEW_ERR(API_BAD_ERROR, "invalid error ID");

namespace tbd {

struct Api {
    Api() = delete;

    using EndpointCallback = uint32_t(*)(const api::Packet&, uint8_t*, size_t&);
    static Error handle_rpc(const api::Packet& request, api::Packet& response, uint8_t* out_buffer, size_t out_buffer_size);

    using EventCallback = uint32_t(*)(const api::Packet&);
    static Error emit_event(const api::Packet& event);

    template<class Message>
    Error emit(const Message& message);

};

}
