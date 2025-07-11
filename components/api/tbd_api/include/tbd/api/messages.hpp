#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/errors.hpp>

namespace tbd::api {

constexpr int32_t NO_MESSAGE = -1;

enum MessageType {
    MESSAGE_TYPE_REQUEST,
    MESSAGE_TYPE_RESPONSE,
    MESSAGE_TYPE_EVENT,
};

struct MessageInfo {
    const char* name;
    MessageType message_type;
    size_t size;
};

}
