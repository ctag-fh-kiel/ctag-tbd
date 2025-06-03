#pragma once

#include <esphome/core/component.h>
#include <esphome/components/uart/uart.h>

#include <tbd/api/packet_stream_parser.hpp>
#include <tbd/api/packet_writers.hpp>


namespace esphome::tbd_uart {

class UARTPacketStream : public uart::UARTDevice {
public:
    size_t queue_size() const {
        return const_cast<UARTPacketStream*>(this)->available();
    }

    bool skip_until(uint8_t value) {
        uint8_t current_value;
        while (available() > 0) {
            if (!peek_byte(&current_value)) {
                return false;
            }
            if (current_value == value) {
                return true;
            }
            if (!read_byte(&value)) {
                return false;
            }
        }
        return false;
    }

    bool take(uint8_t* buffer, size_t num_bytes) {
        return read_array(buffer, num_bytes);
    }

    void put(const uint8_t* buffer, size_t num_bytes) {
        write_array(buffer, num_bytes);
    }

    void send() {
        flush();
    }

private:
    tbd::api::Packet::PacketBuffer buffer_;
};

/** esphome based uart API server
 *
 *  NOTE: The underlying implementation for esp32 uses locking that can not be disabled.
 */
class UARTServer : public UARTPacketStream, public Component {
public:
    UARTServer() : parser_(*this), writer_(*this) {}

    void loop() override;

private:
    tbd::api::Packet::PayloadBuffer payload_buffer_;
    tbd::api::PacketStreamParser<UARTPacketStream> parser_;
    tbd::api::PacketStreamWriter<UARTPacketStream> writer_;
};

}
