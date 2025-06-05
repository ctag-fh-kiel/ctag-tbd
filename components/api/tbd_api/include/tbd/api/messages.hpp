#pragma once

#include <tbd/api/packet.hpp>

namespace tbd::api {

constexpr int32_t NO_MESSAGE = -1;

enum MessageType {
    MESSAGE_TYPE_REQUEST,
    MESSAGE_TYPE_RESPONSE,
};

struct MessageInfo {
    const char* name;
    MessageType message_type;
    size_t size;
};

template<typename Message>
bool decode_message(Message& in_message, const Packet& request);

template<typename Message>
bool encode_message(const Message& out_message, uint8_t* out_buffer, size_t& out_buffer_size);

extern const size_t NUM_REQUEST_MESSAGES;
extern const MessageInfo REQUEST_MESSAGE_LIST[];

extern const size_t NUM_RESPONSE_MESSAGES;
extern const MessageInfo RESPONSE_MESSAGE_LIST[];

}
