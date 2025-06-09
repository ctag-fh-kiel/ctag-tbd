#pragma once

#include <HardwareSerial.h>


#include <tbd/logging.hpp>

#ifndef TBD_API_MAX_PAYLOAD_SIZE
#define TBD_API_MAX_PAYLOAD_SIZE 128
#endif

#include <tbd/api/packet.hpp>
#include <tbd/api/packet_stream_parser.hpp>
#include <tbd/api/packet_writers.hpp>

namespace tbd::client {

constexpr auto tag = "tbd_client";

struct UARTStream {
    explicit UARTStream(
        const uint8_t serial_id,
        const int8_t rx_pin,
        const int8_t tx_pin
    ): rx_pin_(rx_pin), tx_pin_(tx_pin), serial_(serial_id) {}

    void begin(const uint32_t baud_rate) {
        TBD_LOGD(tag, "TBD client pins: BAUD: %i, RX=%i, TX=%i", baud_rate, rx_pin_, tx_pin_);
        serial_.setPins(rx_pin_, tx_pin_);
        serial_.begin(baud_rate);
    }

    size_t queue_size() const {
        const auto bytes = const_cast<HardwareSerial&>(serial_).available();
        TBD_LOGD(tag, "TBD client queue_size: %i", bytes);
        return bytes;
    }

    bool skip_until(const uint8_t value) {
        TBD_LOGD(tag, "TBD client queue_size: %i", serial_.available());
        while (serial_.available() > 0) {
            const char current_value = serial_.peek();
            if (current_value == value) {
                TBD_LOGD(tag, "have start byte");
                return true;
            }
            serial_.read();
        }
        return false;
    }

    bool take(uint8_t* buffer, const size_t num_bytes) {
        TBD_LOGD("tbd_client_demo", "take");
        return serial_.readBytes(buffer, num_bytes);
    }

    void put(const uint8_t* buffer, const size_t num_bytes) {
        TBD_LOGD("tbd_client_demo", "put");
        serial_.write(buffer, num_bytes);
    }

    void send() {
        TBD_LOGD("tbd_client_demo", "send");
        serial_.flush();
    }

private:
    int8_t rx_pin_;
    int8_t tx_pin_;
    HardwareSerial serial_;
};

struct ClientBase {
    ClientBase(const uint8_t serial_id, const uint8_t rx_pin, const uint8_t tx_pin)
        : stream_(serial_id, rx_pin, tx_pin), parser_(stream_), writer_(stream_) {}

    void begin(const uint32_t baud_rate) {
        TBD_LOGI(tag, "TBD client setup");
        stream_.begin(baud_rate);
    }

    void send(const api::Packet& outgoing) {
        writer_.write(outgoing);
    }

    // bool process_incoming() {
    //     if (!parser_.do_work()) {
    //         return false;
    //     }
    //     return true;
    // }

    api::Packet::PayloadBuffer payload_buffer_{};
    UARTStream stream_;
    api::PacketStreamParser<UARTStream> parser_;
    api::PacketStreamWriter<UARTStream> writer_;
};

}