#pragma once

#include <tbd/client/client_base.hpp>


struct TBDClient : protected tbd::client::ClientBase{
    TBDClient(const uint8_t serial_id, const uint8_t rx_pin, const uint8_t tx_pin)
        : ClientBase(serial_id, rx_pin, tx_pin) {}

    void begin(const uint32_t baud_rate) {
        ClientBase::begin(baud_rate);
    }

    bool process_incoming() {
        using tbd::api::Packet;

        if (!parser_.do_work()) {
            return false;
        }
        const auto& incoming = parser_.packet();
        if (incoming.type == Packet::TYPE_RESPONSE) {
            // handle response
        } else if (incoming.type == Packet::TYPE_EVENT) {
            // handle event
        } else {
            TBD_LOGE("tbd_client", "client can not handle %i packets", incoming.type);
        }
        return true;
    }
};
