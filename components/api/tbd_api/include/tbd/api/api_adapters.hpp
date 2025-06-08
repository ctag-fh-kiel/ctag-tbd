#pragma once

#include <tbd/api/packet_parser.hpp>
#include <tbd/api/packet_writers.hpp>
#include <tbd/api/packet_stream_parser.hpp>

#include <tbd/errors.hpp>


namespace tbd::api::impl {

Error handle_rpc(const Packet& request, Packet& response, uint8_t* out_buffer, size_t out_buffer_size);
Error handle_event(const Packet& event);


struct MULTI_THREADED {};
struct RESPONSE_WRITER {};
struct EVENT_WRITER {};

template<class tag, bool allow_multithreading, class output_type>
struct ApiWriter;

template<class tag, class output_type>
struct ApiWriter<tag, false, output_type> {
    size_t write(const Packet& packet) { return inst_.write(packet); }

    const uint8_t* buffer() const { return inst_.buffer(); }
    const std::string_view buffer_view() const { return inst_.buffer_view(); }
    size_t serialized_length() const { return inst_.serialized_length(); }

    /**
     *  allow access to payload buffer for direct writing
     */
    uint8_t* payload_buffer() { return inst_.payload_buffer(); }
    size_t payload_buffer_size() const { return inst_.payload_buffer_size(); }

private:
    static PacketBufferWriter inst_;
};

template<class tag, class output_type>
PacketBufferWriter ApiWriter<tag, false, output_type>::inst_;

template<class tag, class output_type>
struct ApiWriter<tag, true, output_type> : PacketBufferWriter {};

template<class tag, bool allow_multithreading>
using ResponseWriter = ApiWriter<tag, allow_multithreading, EVENT_WRITER>;

template<class tag, bool allow_multithreading>
using EventWriter = ApiWriter<tag, allow_multithreading, EVENT_WRITER>;

template<class WriterT>
size_t handle_and_respond(const Packet& request, uint8_t* output_buffer, size_t output_buffer_size, WriterT& writer) {
    if (request.type == Packet::TYPE_RPC) {
        Packet response;
        if (handle_rpc(request, response, output_buffer, output_buffer_size) != errors::SUCCESS) {
            return 0;
        }
        return writer.write(response);
    } else if (request.type == Packet::TYPE_EVENT) {
        handle_event(request);
    } else {
        TBD_LOGE(tag, "server can not handle %i packets", request.type);
    }
    return 0;
}


template<class tag, bool allow_multithreading>
struct ApiPacketHandler {
    size_t handle(const std::string& input_buffer) {
        const uint8_t* buffer = reinterpret_cast<const uint8_t*>(input_buffer.data());
        return handle(buffer, input_buffer.length());
    }

    size_t handle(const uint8_t* input_buffer, size_t buffer_size) {
        PacketParser parser;
        if (!parser.parse(input_buffer, buffer_size)) {
            return 0;
        }
        return handle_and_respond(parser.packet(), writer_.payload_buffer(), writer_.payload_buffer_size(), writer_);
    }

    const uint8_t* output_buffer() const {
        return writer_.buffer();
    }

    const std::string_view buffer_view() const {
        return writer_.buffer_view();
    }

private:
    ResponseWriter<tag, allow_multithreading> writer_;
};


template<PacketInputStream InputStreamT, PacketOutputStream OutputStreamT>
struct ApiStreamHandler {
    ApiStreamHandler(InputStreamT& input_stream, OutputStreamT& output_stream)
        : parser_(input_stream), writer_(output_stream) {}

    void do_work() {
        if (!parser_.do_work()) {
            return;
        }
        impl::handle_and_respond(parser_.packet(), payload_buffer_, Packet::MAX_PAYLOAD_SIZE, writer_);
    }

private:
    Packet::PayloadBuffer payload_buffer_;
    PacketStreamParser<InputStreamT> parser_;
    PacketStreamWriter<OutputStreamT> writer_;
};

}
