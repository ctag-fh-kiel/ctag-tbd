#pragma once

#include <tbd/api.hpp>

#include <cinttypes>
#include <cstdlib>

namespace tbd::api {



constexpr int32_t NO_MESSAGE = -1;

struct Endpoint {
    const char* path;
    int32_t request_type;
    int32_t response_type;
    Api::EndpointCallback callback;
};

enum MessageType {
    MESSAGE_TYPE_REQUEST,
    MESSAGE_TYPE_RESPONSE,
};

struct MessageInfo {
    const char* name;
    MessageType message_type;
    size_t size;
};

extern const size_t NUM_ENDPOINTS;
extern const Endpoint ENDPOINT_LIST[];

extern const size_t NUM_REQUEST_MESSAGES;
extern const MessageInfo REQUEST_MESSAGE_LIST[];

extern const size_t NUM_RESPONSE_MESSAGES;
extern const MessageInfo RESPONSE_MESSAGE_LIST[];

template<typename Message> 
bool decode_message(Message& in_message, const uint8_t* in_buffer, size_t len);

template<typename Message>
bool encode_message(const Message& out_message, uint8_t* buffer, size_t& buffer_size);

}
