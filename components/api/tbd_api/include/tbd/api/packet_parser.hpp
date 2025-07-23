#pragma once
#include <tbd/api/module.hpp>

#include <tbd/api/packet.hpp>
#include <tbd/logging.hpp>
#include <tbd/ram.hpp>


namespace tbd::api {

/**
 *   Reusable package parser that caches the last parsed package.
 */
struct PacketBufferParser {
    PacketBufferParser(const uint8_t* buffer, const size_t buffer_size) : buffer_(buffer), buffer_size_(buffer_size) {}

    const Packet& packet() const { return packet_; }

    bool parse() { return _parse(); }
    bool parse_header() { return _parse_header(); }
    Packet::PacketType type_peek() { return _peek_header(); }

    uint8_t* input_buffer() { return const_cast<uint8_t*>(buffer_); }
    size_t buffer_size() const { return buffer_size_; }

protected:
    bool _parse() {
        if (buffer_size_ < Packet::HEADER_SIZE) {
            TBD_LOGE(tag, "input length insufficient for request header");
            return false;
        }
        if (!parse_header()) {
            return false;
        }
        if (buffer_size_ < packet_.total_size()) {
            TBD_LOGE(tag, "input length insufficient for request");
            return false;
        }
        packet_.payload = buffer_ + Packet::HEADER_SIZE;
        if (const uint8_t end_byte = buffer_[Packet::OFFSET_HEADER_END + packet_.payload_length + 1];
            end_byte != Packet::END_BYTE)
        {
            TBD_LOGE(tag, "invalid packet end byte %i", end_byte);
            return false;
        }
        return true;
    }

    Packet::PacketType _peek_header() {
        if (buffer_size_ < Packet::HEADER_SIZE || !read_start() || !read_type()) {
            return Packet::TYPE_INVALID;
        }
        return packet_.type;
    }

    bool _parse_header() {
        return read_start()
               && read_type()
               && read_handler()
               && read_crc()
               && read_length()
               && read_id()
               && read_header_end();
    }

    bool read_start() const {
        const uint8_t start_byte = buffer_[Packet::OFFSET_START];
        if (start_byte == Packet::START_BYTE) {
            return true;
        }
        TBD_LOGE(tag, "expected start byte, got %i", start_byte);
        return false;
    }

    bool read_type() {
        const uint8_t type = buffer_[Packet::OFFSET_TYPE] & Packet::TYPE_MASK;
        if (type < Packet::TYPE_INVALID) {
            packet_.type = static_cast<Packet::PacketType>(type);
            return true;
        }
        TBD_LOGE(tag, "expected type byte, got %i", type);
        return false;
    }

    bool read_handler() {
        const uint8_t* handler_bytes = buffer_ + Packet::OFFSET_HANDLER_LOW;
        const uint16_t handler = handler_bytes[0] + (handler_bytes[1] << 8);
        packet_.handler = handler;
        return true;
    }

    bool read_crc() const {
        const uint8_t crc = buffer_[Packet::OFFSET_CRC];
        if (crc == 0) {
            return true;
        }
        TBD_LOGE(tag, "disabled CRC byte should be 0, got %i", crc);
        return false;
    }

    bool read_length() {
        const uint8_t* length_bytes = buffer_ + Packet::OFFSET_LENGTH_LOW;
        const uint16_t length = length_bytes[0] + (length_bytes[1] << 8);

        if (length > Packet::MAX_PAYLOAD_SIZE) {
            TBD_LOGE(tag, "request payload size %i exceeds max of %i", length, Packet::MAX_PAYLOAD_SIZE);
            return false;
        }

        packet_.payload_length = length;
        return true;
    }

    bool read_id() {
        const uint8_t* id_bytes = buffer_ + Packet::OFFSET_ID_LOW;
        const uint16_t id = id_bytes[0] + (id_bytes[1] << 8);
        packet_.id = id;
        return true;
    }

    bool read_header_end() const {
        const uint8_t header_end_byte = buffer_[Packet::OFFSET_HEADER_END];
        if (header_end_byte == Packet::HEADER_END_BYTE) {
            return true;
        }
        TBD_LOGE(tag, "expected payload byte, got %i", header_end_byte);
        return false;
    }

    Packet packet_;
    const uint8_t* buffer_;
    const size_t buffer_size_;
};

template<class tag>
struct StaticBufferParser: PacketBufferParser {
    StaticBufferParser(): PacketBufferParser(owned_buffer_, Packet::BUFFER_SIZE) {}
    static TBD_DMA Packet::PacketBuffer owned_buffer_;
};

template<class tag>
Packet::PacketBuffer StaticBufferParser<tag>::owned_buffer_;

template<class tag>
struct StackBufferParser: PacketBufferParser {
    StackBufferParser(): PacketBufferParser(owned_buffer_, Packet::BUFFER_SIZE) {}

    Packet::PacketBuffer owned_buffer_;
};

}
