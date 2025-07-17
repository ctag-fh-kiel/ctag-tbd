#include <tbd/api/api_adapters.hpp>
#include <tbd/api/module.hpp>

#include <tbd/api/endpoint_index.hpp>
#include <tbd/api/event_index.hpp>
#include <tbd/logging.hpp>


namespace tbd::api::impl {
    
Error handle_rpc(const Packet& request, Packet& response, uint8_t* out_buffer, const size_t out_buffer_size) {
    if (request.type != Packet::TYPE_RPC) {
        TBD_LOGE(tag, "request type is not RPC: %i", request.type);
        response.set_error(TBD_ERR(API_WRONG_PACKET_TYPE), request.id);
        return TBD_OK;
    }

    const auto endpoint_id = request.handler;
    if (endpoint_id >= api::NUM_ENDPOINTS) {
        TBD_LOGE(tag, "invalid endpoint: %i", endpoint_id);
        response.set_error(TBD_ERR(API_BAD_ENDPOINT), request.id);
        return TBD_OK;
    }

    const auto& endpoint = ENDPOINT_LIST[endpoint_id];
    if (const auto response_type_id = endpoint.response_type; response_type_id != NO_MESSAGE) {
        if (const auto required_buffer_size = RESPONSE_MESSAGE_LIST[response_type_id].size;
            required_buffer_size > 0 && required_buffer_size > out_buffer_size)
        {
            TBD_LOGE(tag, "api response buffer for endpoint %i has insufficient size: required %i, provided %i",
                     endpoint_id, required_buffer_size, out_buffer_size);
            response.set_error(TBD_ERR(API_RESPONSE_BUFFER_SIZE), request.id);
            return TBD_OK;
        }
    }

    const auto handler = endpoint.callback;

    size_t length = out_buffer_size;
    if (const auto err = handler(request, out_buffer, length); err != TBD_OK) {
        TBD_LOGE(tag, "handler for endpoint %i failed", endpoint_id);
        response.set_error(err, request.id);
        return TBD_OK;
    }
    response.type = Packet::TYPE_RESPONSE;
    response.handler = 0;
    response.payload_length = length;
    response.id = request.id;
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
