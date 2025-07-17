#pragma once
#include <tbd/api/module.hpp>

#include <tbd/api/packet.hpp>
#include <tbd/logging.hpp>

#include <string>


namespace tbd::api {

struct PacketParser {
    const Packet& packet() const { return packet_; }

    bool parse(const uint8_t* buffer, size_t buffer_size) { return _parse_from_buffer(buffer, buffer_size); }
    bool parse_header(const uint8_t* buffer) { return _parse_header(buffer); }

protected:
    bool _parse_from_buffer(const uint8_t* buffer, size_t buffer_size) {
        if (buffer_size < Packet::HEADER_SIZE) {
            TBD_LOGE(tag, "input length insufficient for request header");
            return false;
        }
        if (!parse_header(buffer)) {
            return false;
        }
        if (buffer_size < packet_.total_size()) {
            TBD_LOGE(tag, "input length insufficient for request");
            return false;
        }
        packet_.payload = buffer + Packet::HEADER_SIZE;
        uint8_t end_byte = buffer[Packet::OFFSET_HEADER_END + packet_.payload_length + 1];
        if (end_byte != Packet::END_BYTE) {
            TBD_LOGE(tag, "invalid packet end byte %i", end_byte);
            return false;
        }
        return true;
    }

    bool _parse_header(const uint8_t* buffer) {
        return read_start(buffer)
               && read_type(buffer)
               && read_handler(buffer)
               && read_crc(buffer)
               && read_length(buffer)
               && read_id(buffer)
               && read_header_end(buffer);
    }

    bool read_start(const uint8_t* buffer) {
        uint8_t start_byte = buffer[Packet::OFFSET_START];
        if (start_byte == Packet::START_BYTE) {
            return true;
        }
        TBD_LOGE(tag, "expected start byte, got %i", start_byte);
        return false;
    }

    bool read_type(const uint8_t* buffer) {
        uint8_t type = buffer[Packet::OFFSET_TYPE] & Packet::TYPE_MASK;
        if (type < Packet::TYPE_INVALID) {
            packet_.type = static_cast<Packet::PacketType>(type);
            return true;
        }
        TBD_LOGE(tag, "expected type byte, got %i", type);
        return false;
    }

    bool read_handler(const uint8_t* buffer) {
        const uint8_t* handler_bytes = buffer + Packet::OFFSET_HANDLER_LOW;
        uint16_t handler = handler_bytes[0] + (handler_bytes[1] << 8);
        packet_.handler = handler;
        return true;
    }

    bool read_crc(const uint8_t* buffer) {
        uint8_t crc = buffer[Packet::OFFSET_CRC];
        if (crc == 0) {
            return true;
        }
        TBD_LOGE(tag, "disabled CRC byte should be 0, got %i", crc);
        return false;
    }

    bool read_length(const uint8_t* buffer) {
        const uint8_t* length_bytes = buffer + Packet::OFFSET_LENGTH_LOW;
        uint16_t length = length_bytes[0] + (length_bytes[1] << 8);

        if (length > Packet::MAX_PAYLOAD_SIZE) {
            TBD_LOGE(tag, "request payload size %i exceeds max of %i", length, Packet::MAX_PAYLOAD_SIZE);
            return false;
        }

        packet_.payload_length = length;
        return true;
    }

    bool read_id(const uint8_t* buffer) {
        const uint8_t* id_bytes = buffer + Packet::OFFSET_ID_LOW;
        uint16_t id = id_bytes[0] + (id_bytes[1] << 8);
        packet_.id = id;
        return true;
    }

    bool read_header_end(const uint8_t* buffer) {
        uint8_t header_end_byte = buffer[Packet::OFFSET_HEADER_END];
        if (header_end_byte == Packet::HEADER_END_BYTE) {
            return true;
        }
        TBD_LOGE(tag, "expected payload byte, got %i", header_end_byte);
        return false;
    }

    Packet packet_;
};

}
