#include <tbd/api.hpp>
#include <tbd/api/module.hpp>

#include <tbd/api/api_all_endpoints.hpp>
#include <tbd/logging.hpp>

#include <pb_decode.h>
#include <api_types.pb.h>

using tbd::api::tag;

namespace tbd {
    
uint32_t Api::handle_stream_input(uint8_t* buffer, size_t& length) {
    void_request header;
    pb_istream_t header_stream = pb_istream_from_buffer(buffer, void_request_size);
    pb_decode(&header_stream, void_request_fields, &header);

    // FIXME: the parser does not ignore unknown fields as expected
    // if (!pb_decode(&header_stream, void_request_fields, &header)) {
    //     TBD_LOGE("api", "failed to deserialize header: %s", PB_GET_ERROR(&header_stream));
    //     return TBD_ERR(API_BAD_HEADER);
    // }

    pb_istream_t stream = pb_istream_from_buffer(buffer, length);
    if (header.endpoint >= api::NUM_ENDPOINTS) {
        return TBD_ERR(API_BAD_ENDPOINT);
    }
    const auto& endpoint = api::ENDPOINT_LIST[header.endpoint];
    const auto request_type_id = endpoint.request_type;
    const auto response_type_id = endpoint.response_type;

    size_t required_buffer_size = 0;
    if (request_type_id != api::NO_MESSAGE) {
        const auto request_size = api::REQUEST_MESSAGE_LIST[request_type_id].size;
        required_buffer_size = request_size > required_buffer_size ? request_size : required_buffer_size;
    }
    if (response_type_id != api::NO_MESSAGE) {
        const auto response_size = api::RESPONSE_MESSAGE_LIST[response_type_id].size;
        required_buffer_size = response_size > required_buffer_size ? response_size : required_buffer_size;
    }
    if (length < required_buffer_size) {
        return TBD_ERR(API_BUFFER_SIZE);
    }
    
    auto handler = endpoint.callback;
    return handler(buffer, length);
}

}