#pragma once

#include <tbd/api/packet_parser.hpp>
#include <tbd/api/packet_writers.hpp>
#include <tbd/api/packet_stream_parser.hpp>

#include <tbd/errors.hpp>


namespace tbd::api::impl {

// plain API processing functions //

Error handle_rpc(const Packet& request, Packet& response, uint8_t* out_buffer, size_t out_buffer_size);
Error handle_event(const Packet& event);


// API adapter configuration tags //

enum class ApiOutputType {
    RESPONSE_WRITER,
    EVENT_WRITER,
};

// primary API implementation helpers //

/** Convenience class for API channel implementation using buffers.
 *
 * @tparam channel_tag            name tag for the API channel using the packet handler
 * @tparam allow_multithreading   resort to stack based output buffer if API handling is multithreaded
 */
template<class channel_tag, bool allow_multithreading>
struct ApiPacketHandler;

/** Convenience class for API channel implementation using streams.
 *
 * @param InputStreamT    stream to read from
 * @param OutputStreamT   stream to write to
 */
template<PacketInputStream InputStreamT, PacketOutputStream OutputStreamT>
struct ApiStreamHandler;

// secondary

template<class channel_tag, bool allow_multithreading, ApiOutputType output_type>
struct ApiWriter;

template<class channel_tag, ApiOutputType output_type>
struct ApiWriter<channel_tag, false, output_type> : StaticBufferWriter<channel_tag> {};

template<class channel_tag, ApiOutputType output_type>
struct ApiWriter<channel_tag, true, output_type> : StackBufferWriter<channel_tag> {};

template<class channel_tag, bool allow_multithreading>
struct ApiReader;

template<class channel_tag>
struct ApiReader<channel_tag, false> : StaticBufferParser<channel_tag> {};

template<class channel_tag>
struct ApiReader<channel_tag, true> : StackBufferParser<channel_tag> {};

template<class channel_tag, bool allow_multithreading>
using ResponseWriter = ApiWriter<channel_tag, allow_multithreading, ApiOutputType::RESPONSE_WRITER>;

template<class channel_tag, bool allow_multithreading>
using EventWriter = ApiWriter<channel_tag, allow_multithreading, ApiOutputType::EVENT_WRITER>;


// implementations //

template<class WriterT>
size_t handle_and_respond(const Packet& request, uint8_t* output_buffer, size_t output_buffer_size, WriterT& writer) {
    switch (request.type) {
        case Packet::TYPE_NOOP: {
            break;
        }
        case Packet::TYPE_RPC: {
            Packet response;
            if (handle_rpc(request, response, output_buffer, output_buffer_size) != errors::SUCCESS) {
                break;
            }
            return writer.write(response);
        }
        case Packet::TYPE_EVENT: {
            handle_event(request);
        }
        default: {
            TBD_LOGE(tag, "server can not handle %i packets", request.type);
        }
    }
    return 0;
}


template<class channel_tag, bool allow_multithreading>
struct ApiPacketHandler {
    size_t handle(const std::string& input_buffer) {
        const auto* buffer = reinterpret_cast<const uint8_t*>(input_buffer.data());
        return handle(buffer, input_buffer.length());
    }

    size_t handle(const uint8_t* input_buffer, size_t buffer_size) {
        PacketBufferParser parser(input_buffer, buffer_size);
        if (!parser.parse()) {
            return 0;
        }
        return handle_and_respond(parser.packet(), writer_.payload_buffer(), writer_.payload_buffer_size(), writer_);
    }

    const uint8_t* output_buffer() const {
        return writer_.buffer();
    }

    #ifdef __cpp_lib_string_view
    const std::string_view buffer_view() const {
        return writer_.buffer_view();
    }
    #endif

private:
    ResponseWriter<channel_tag, allow_multithreading> writer_;
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
