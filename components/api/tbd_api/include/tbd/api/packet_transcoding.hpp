#pragma once

#include <tbd/serialization/dtos/messages.hpp>
#include <tbd/api/dtos/messages.hpp>
#include <tbd/api/messages.hpp>

namespace tbd::api {

template<typename MessageT>
Error decode_packet(MessageT& in_message, const Packet& packet) {
    pb_istream_t stream = pb_istream_from_buffer(packet.payload, packet.payload_length);
    return serialization::decode_message<MessageT>(in_message, stream);
}

template<typename MessageT>
Error encode_packet(const MessageT& out_message, uint8_t* out_buffer, size_t& out_buffer_size) {
    pb_ostream_t stream = pb_ostream_from_buffer(out_buffer, out_buffer_size);
    return serialization::encode_message<MessageT>(out_message, stream);
}

}
