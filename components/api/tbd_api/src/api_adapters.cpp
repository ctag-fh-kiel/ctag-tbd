#include <tbd/api/api_adapters.hpp>
#include <tbd/api/module.hpp>

#include <tbd/api/api_all_endpoints.hpp>
#include <tbd/api/api_all_events.hpp>
#include <tbd/logging.hpp>


namespace tbd::api::impl {
    
Error handle_rpc(const Packet& request, Packet& response, uint8_t* out_buffer, size_t out_buffer_size) {
    if (request.type != Packet::TYPE_RPC) {
        TBD_LOGE(tag, "request type is not RPC: %i", request.type);
        return TBD_ERR(API_WRONG_PACKET_TYPE);
    }

    const auto endpoint_id = request.handler;
    if (endpoint_id >= api::NUM_ENDPOINTS) {
        TBD_LOGE(tag, "invalid endpoint: %i", endpoint_id);
        return TBD_ERR(API_BAD_ENDPOINT);
    }

    const auto& endpoint = api::ENDPOINT_LIST[endpoint_id];
    const auto response_type_id = endpoint.response_type;


    if (const auto required_buffer_size = api::RESPONSE_MESSAGE_LIST[response_type_id].size;
        response_type_id != api::NO_MESSAGE && required_buffer_size > out_buffer_size)
    {
        TBD_LOGE(tag, "api response buffer for endpoint %i has insufficient size: required %i, provided %i",
                 endpoint_id, required_buffer_size, out_buffer_size);
        response.type = Packet::TYPE_RESPONSE;
        response.handler = TBD_ERR(API_RESPONSE_BUFFER_SIZE);
        response.payload_length = 0;
        response.payload = nullptr;
        return errors::SUCCESS;
    }

    response.id = request.id;
    response.crc = 0;
    const auto handler = endpoint.callback;

    size_t length = out_buffer_size;
    if (const auto err = handler(request, out_buffer, length); err != errors::SUCCESS) {
        TBD_LOGE(tag, "handler for endpoint %i failed", endpoint_id);
        response.type = Packet::TYPE_ERROR;
        response.handler = err;
        response.payload_length = 0;
        response.payload = nullptr;
        return TBD_OK;
    }
    response.type = Packet::TYPE_RESPONSE;
    response.handler = 0;
    response.payload_length = length;
    response.payload = out_buffer;

    return TBD_OK;
}

Error handle_event(const Packet& request) {
    if (request.type != Packet::TYPE_EVENT) {
        TBD_LOGE(tag, "request type is not event: %i", request.type);
        return TBD_ERR(API_WRONG_PACKET_TYPE);
    }

    const auto event_id = request.handler;
    if (event_id >= api::NUM_EVENTS) {
        TBD_LOGE(tag, "invalid event: %i", event_id);
        return TBD_ERR(API_BAD_EVENT);
    }

    auto& event = api::EVENT_LIST[event_id];
    const auto handler = event.callback;

    if (const auto err = handler(request); err != errors::SUCCESS) {
        TBD_LOGE(tag, "handler for event %i failed", event_id);
    }
    return TBD_OK;
}

}
