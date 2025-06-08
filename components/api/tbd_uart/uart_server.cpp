#include "uart_server.hpp"

#include <tbd/logging.hpp>
#include <tbd/errors.hpp>


using tbd::api::tag;
using tbd::api::Packet;

namespace esphome::tbd_uart {

void UARTServer::loop() {
    stream_handler_.do_work();
}

}
