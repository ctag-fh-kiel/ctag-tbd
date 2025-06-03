#include "uart_server.hpp"

#include <tbd/logging.hpp>
#include <tbd/errors.hpp>


using tbd::api::tag;
using tbd::api::Packet;

namespace esphome::tbd_uart {

void UARTServer::loop() {
    if (!parser_.do_work()) {
        return;
    }

    auto& request = parser_.packet();
    Packet response;
    if (auto err = tbd::Api::handle_rpc(request, response, payload_buffer_, Packet::MAX_PAYLOAD_SIZE); err != tbd::errors::SUCCESS) {
        TBD_LOGE(tag, "request failed");
        return;
    }
    writer_.send(response);
}

}
