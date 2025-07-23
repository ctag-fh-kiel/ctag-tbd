#pragma once

#include <tbd/api/packet.hpp>

#include "../../../../../../../../components/core/tbd_system/esp32/include/tbd/system/port/ram.hpp"

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

};

/** Convenience class to serialize packages to output buffer for sending.
 *
 *  Class wraps an output buffer, that
 *
 *  - can be accessed for sending
 *  - payload section can be accessed for external payload serialization
 *
 */
struct PacketBufferWriter {
    explicit PacketBufferWriter(uint8_t* buffer, const size_t buffer_size)
        : serialized_length_(0), buffer_size_(buffer_size), buffer_(buffer) {}

    size_t write(const Packet& packet) {
        return _write(packet);
    }

    const uint8_t* buffer() const { return buffer_; }
    size_t buffer_size() const { return buffer_size_; }

    size_t serialized_length() const { return serialized_length_; }

#ifdef __cpp_lib_string_view
    const std::string_view buffer_view() const {
        return std::string_view(reinterpret_cast<const char*>(buffer_), serialized_length_);
    }
#endif

    /**
     *  allow access to payload buffer for direct writing
     */
    uint8_t* payload_buffer() const { return buffer_ + Packet::HEADER_SIZE; }
    size_t payload_buffer_size() const { return buffer_size_ - (Packet::HEADER_SIZE + Packet::END_SIZE); }

protected:

    size_t _write(const Header& packet) {
        impl::write_packet_header_to_buffer(packet, buffer_);
        buffer_[Packet::OFFSET_HEADER_END + packet.payload_length + 1] = Packet::END_BYTE;
        serialized_length_ = packet.total_size();
        return serialized_length_;
    }

    size_t serialized_length_;
    const size_t buffer_size_;
    uint8_t* buffer_;
};

template<class tag>
struct StaticBufferWriter: PacketBufferWriter {
    StaticBufferWriter(): PacketBufferWriter(owned_buffer_, Packet::BUFFER_SIZE) {}
    static TBD_DMA Packet::PacketBuffer owned_buffer_;
};

template<class tag>
Packet::PacketBuffer StaticBufferWriter<tag>::owned_buffer_;

template<class tag>
struct StackBufferWriter: PacketBufferWriter {
    StackBufferWriter(): PacketBufferWriter(owned_buffer_, Packet::BUFFER_SIZE) {}

    Packet::PacketBuffer owned_buffer_;
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


/** Convenience class to serialize packages to stream for sending.
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
