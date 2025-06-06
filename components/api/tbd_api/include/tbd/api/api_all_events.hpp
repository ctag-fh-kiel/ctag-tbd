#pragma once

#include <tbd/api.hpp>
#include <tbd/api/packet.hpp>
#include <tbd/api/messages.hpp>


namespace tbd::api {

struct Event {
    const char* path;
    int32_t event_type;
    Api::EventCallback callback;
};

extern const size_t NUM_EVENTS;
extern const Event EVENT_LIST[];

extern const size_t NUM_EVENT_MESSAGES;
extern const MessageInfo EVENT_MESSAGE_LIST[];

}
