#include "uart_server.hpp"

#include <tbd/logging.hpp>
#include <tbd/errors.hpp>


using tbd::api::tag;
using tbd::api::Packet;

namespace esphome::tbd_uart {

void UARTServer::loop() {
    stream_handler_.do_work();
}

UARTServer* UARTServer::inst_;

[[tbd::sink]]
void emit_on_uart(const uint8_t* buffer, size_t length) {
    if (UARTServer::inst_ == nullptr) {
        TBD_LOGE(tag, "uart server instance not initialized");
        return;
    }
    UARTServer::inst_->put(buffer, length);
    UARTServer::inst_->send();
}

}
