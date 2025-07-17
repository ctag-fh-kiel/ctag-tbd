#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/api/messages.hpp>
#include <tbd/errors.hpp>


namespace tbd::api {

using EventCallback = Error(*)(const api::Packet&);

struct Event {
    const char* path;
    int32_t event_type;
    EventCallback callback;
};

extern const uint16_t NUM_EVENTS;
extern const Event EVENT_LIST[];

extern const size_t NUM_EVENT_MESSAGES;
extern const MessageInfo EVENT_MESSAGE_LIST[];

}
