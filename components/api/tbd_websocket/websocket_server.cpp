#include "websocket_server.hpp"

#include <tbd/api/websocket_server.hpp>

namespace esphome::tbd_websocket {

WebsocketServer::WebsocketServer() {

}

void WebsocketServer::setup() {
    tbd::api::websocket_server::begin();
}

float WebsocketServer::get_setup_priority() const {
    return setup_priority::AFTER_WIFI;
}

void WebsocketServer::on_shutdown() {
    tbd::api::websocket_server::end();
}

}
