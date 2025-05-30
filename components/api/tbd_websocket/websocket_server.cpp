#include "websocket_server.hpp"

#include <tbd/api/websocket_server.hpp>

namespace esphome::tbd_websocket {

WebsocketServer::WebsocketServer() {

}

void WebsocketServer::setup() {
    tbd::api::websocket_server::begin();
}
//  uint16_t get_port() const;
float WebsocketServer::get_setup_priority() const {
    return setup_priority::AFTER_WIFI;
}
//  void loop() override;
//  void dump_config() override;
void WebsocketServer::on_shutdown() {
    tbd::api::websocket_server::end();
}

}
