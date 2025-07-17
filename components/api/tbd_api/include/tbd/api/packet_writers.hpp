#pragma once

#include <tbd/api/packet.hpp>

#ifdef __cpp_concepts
#include <concepts>
#endif


namespace tbd::api {

namespace impl {

inline void write_packet_header_to_buffer(const Header& packet, uint8_t* buffer) {
    buffer[Packet::OFFSET_START] = Packet::START_BYTE;
    buffer[Packet::OFFSET_TYPE] = packet.type;
    buffer[Packet::OFFSET_HANDLER_LOW] = packet.handler & 0xff;
    buffer[Packet::OFFSET_HANDLER_HIGH] = (packet.handler >> 8) & 0xff;
    buffer[Packet::OFFSET_CRC] = 0;
    buffer[Packet::OFFSET_LENGTH_LOW] = packet.payload_length & 0xff;
    buffer[Packet::OFFSET_LENGTH_HIGH] = (packet.payload_length >> 8) & 0xff;
    buffer[Packet::OFFSET_ID_LOW] = packet.id & 0xff;
    buffer[Packet::OFFSET_ID_HIGH] = (packet.id >> 8) & 0xff;
    buffer[Packet::OFFSET_HEADER_END] = Packet::HEADER_END_BYTE;
}

}

/** Packet writer for packet based communication.
 *
 *  PacketBufferWriter writes packages to a single buffer, for later transfer via a communication channel that has its
 *  own package semantics. This allows the transfer of TBD packages as a single integral communication packet.
 *
 *
 *
 */
struct PacketBufferWriter {
    size_t write(const Packet& packet) {
        return _write(packet);
    }

    const uint8_t* buffer() const { return buffer_; }

    #ifdef __cpp_lib_string_view
    const std::string_view buffer_view() const {
        return std::string_view(reinterpret_cast<const char*>(buffer_), serialized_length_);
    }
    #endif

    size_t serialized_length() const { return serialized_length_; }

    /**
     *  allow access to payload buffer for direct writing
     */
    uint8_t* payload_buffer() { return buffer_ + Packet::HEADER_SIZE; }
    size_t payload_buffer_size() const { return Packet::MAX_PAYLOAD_SIZE; }

private:

    size_t _write(const Header& packet) {
        impl::write_packet_header_to_buffer(packet, buffer_);
        buffer_[Packet::OFFSET_HEADER_END + packet.payload_length + 1] = Packet::END_BYTE;
        serialized_length_ = packet.total_size();
        return serialized_length_;
    }

    size_t serialized_length_;
    Packet::PacketBuffer buffer_;
};

#ifdef __cpp_concepts
/** Required concept for stream classes used by PacketStreamWriter
 *
 */
template<class ImplT>
concept PacketOutputStream = requires(ImplT& impl, size_t _size_t, const uint8_t* _const_uint8_t_ptr) {
    { impl.put(_const_uint8_t_ptr, _size_t) } -> std::same_as<void>;
    { impl.send() } -> std::same_as<void>;
};
#else
#define PacketOutputStream class
#endif


/** Packet writer for output streams.
 *
 *  Using PacketStreamWriter avoids unnecessary payload copying and writes packages to stream in three chunks:
 *
 *  - header block
 *  - payload
 *  - end block
 *
 */
template<PacketOutputStream StreamT>
struct PacketStreamWriter {
    PacketStreamWriter(StreamT& stream) : stream_(stream) {}

    size_t write(const Packet& packet) {
        return _send(packet);
    }

private:
    size_t _send(const Packet& packet) {
        Packet::HeaderBuffer header_buffer;
        impl::write_packet_header_to_buffer(packet, header_buffer);
        stream_.put(header_buffer, Packet::HEADER_SIZE);

        if (packet.payload_length > 0) {
            stream_.put(packet.payload, packet.payload_length);
        }
        uint8_t end_byte = Packet::END_BYTE;
        stream_.put(&end_byte, 1);
        stream_.send();
        return packet.total_size();
    }

    StreamT& stream_;
};

#ifdef PacketOutputStream
#undef PacketOutputStream
#endif

}
