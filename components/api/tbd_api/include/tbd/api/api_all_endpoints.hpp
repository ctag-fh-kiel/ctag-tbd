#pragma once

#include <tbd/api.hpp>
#include <tbd/api/packet.hpp>
#include <tbd/api/messages.hpp>


namespace tbd::api {

struct Endpoint {
    const char* path;
    int32_t request_type;
    int32_t response_type;
    Api::EndpointCallback callback;
};

extern const uint32_t API_VERSION;
extern const uint32_t CORE_API_HASH;
extern const uint32_t BASE_API_HASH;
extern const uint32_t API_HASH;

extern const size_t NUM_ENDPOINTS;
extern const Endpoint ENDPOINT_LIST[];

extern const size_t NUM_REQUEST_MESSAGES;
extern const MessageInfo REQUEST_MESSAGE_LIST[];

extern const size_t NUM_RESPONSE_MESSAGES;
extern const MessageInfo RESPONSE_MESSAGE_LIST[];

}
