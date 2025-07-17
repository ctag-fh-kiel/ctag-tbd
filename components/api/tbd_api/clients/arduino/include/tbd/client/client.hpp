#pragma once

#include <HardwareSerial.h>

#include <tbd/client/module.hpp>
#include <tbd/logging.hpp>

#include <functional>


#ifndef TBD_API_MAX_PAYLOAD_SIZE
#define TBD_API_MAX_PAYLOAD_SIZE 128
#endif

#ifndef TBD_CLIENT_MAX_PENDING_REQUESTS
#define TBD_CLIENT_MAX_PENDING_REQUESTS 10
#endif

#include <tbd/api/packet.hpp>
#include <tbd/api/packet_stream_parser.hpp>
#include <tbd/api/packet_writers.hpp>

namespace tbd::client {

struct TBDClient;

struct UARTConfig {
    uint8_t serial_id;
    int8_t rx_pin;
    int8_t tx_pin;
    uint32_t baud_rate;
};

/** Interface adapter for reader and writer.
 *
 *  This interface adapter converts an Arduino HardwareSerial device to the access interface expected by both
 *  tbd::api::PacketStreamParser and tbd::api::PacketOutputStream.
 *
 */
struct UARTStream {
    explicit UARTStream(const UARTConfig& config): config_(config), serial_(config.serial_id) {}

    void begin() {
        TBD_LOGD(tag, "TBD client pins: BAUD: %i, RX=%i, TX=%i", config_.baud_rate, config_.rx_pin, config_.tx_pin);
        serial_.setPins(config_.rx_pin, config_.tx_pin);
        serial_.begin(config_.baud_rate);
    }

    size_t queue_size() const {
        const auto bytes = const_cast<HardwareSerial&>(serial_).available();
        return bytes;
    }

    bool skip_until(const uint8_t value) {
        while (serial_.available() > 0) {
            const char current_value = serial_.peek();
            if (current_value == value) {
                return true;
            }
            serial_.read();
        }
        return false;
    }

    bool take(uint8_t* buffer, const size_t num_bytes) {
        return serial_.readBytes(buffer, num_bytes);
    }

    void put(const uint8_t* buffer, const size_t num_bytes) {
        serial_.write(buffer, num_bytes);
    }

    void send() {
        serial_.flush();
    }

private:
    UARTConfig config_;
    HardwareSerial serial_;
};


#ifdef __cpp_concepts
/** Required concept for stream classes used by PacketStreamWriter
 *
 */
template<class ImplT>
concept PacketReceiver = requires(ImplT& impl, const Packet& packet) {
    { impl.handle_incoming(packet) } -> std::same_as<void>;
};
#else
#define PacketReceiver class
#endif


struct ClientBase {
    using ResponseHandler = std::function<void(const api::Packet&)>;

    explicit ClientBase(const UARTConfig& uart_config) : stream_(uart_config), writer_(stream_) {}

    void begin(const uint32_t) {
        TBD_LOGI(tag, "TBD client setup");
        stream_.begin();
    }

    void send(const api::Packet& outgoing) {
        writer_.write(outgoing);
    }

    Error send(api::Packet& outgoing, ResponseHandler&& handler) {
        for (size_t i = 0; i < TBD_CLIENT_MAX_PENDING_REQUESTS; ++i) {
            if (pending_requests_[i]) {
                continue;
            }
            pending_requests_[i] = std::move(handler);
            outgoing.id = i;
            writer_.write(outgoing);
            return TBD_OK;
        }
        return TBD_ERR(CLIENT_MAX_PENDING_REQUESTS);
    }

protected:
    ResponseHandler pending_requests_[TBD_CLIENT_MAX_PENDING_REQUESTS];
    api::Packet::PayloadBuffer payload_buffer_;
    UARTStream stream_;
    api::PacketStreamWriter<UARTStream> writer_;
};


template<PacketReceiver RpcT, PacketReceiver EventT>
struct Client : ClientBase {


    explicit Client(const UARTConfig& uart_config)
        : ClientBase(uart_config), rpc(*this), event(*this), parser_(stream_) {}

    void begin() {
        TBD_LOGI(tag, "TBD client setup");
        stream_.begin();
    }

    // Error send(const api::Packet& outgoing) {
    //     writer_.write(outgoing);
    //     return TBD_OK;
    // }



    /** Check for and process new incoming messages.
     *
     * Call this method from a worker loop to respond to requests or do not use for send/dispatch only clients.
     *
     * @return signal if new package has been handled
     */
    bool process_incoming() {
        using api::Packet;

        if (!parser_.do_work()) {
            return false;
        }
        const auto& incoming = parser_.packet();
        TBD_LOGI(tag, "got packet");
        if (incoming.type == Packet::TYPE_RESPONSE) {
            const auto id = incoming.id;
            if (id >= TBD_CLIENT_MAX_PENDING_REQUESTS || !pending_requests_[id]) {
                TBD_LOGI(tag, "bad request ID %i in response", id);
            }
            pending_requests_[id](incoming);
            pending_requests_[id] = {};

        } else if (incoming.type == Packet::TYPE_EVENT) {
            TBD_LOGI(tag, "got event");
            event.handle_event(incoming);
        } else {
            TBD_LOGE("tbd_client", "client can not handle %i packets", incoming.type);
            return false;
        }
        return true;
    }

    RpcT rpc;
    EventT event;

private:

    api::PacketStreamParser<UARTStream> parser_;
};

}