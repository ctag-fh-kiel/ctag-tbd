#include "websocket_server.hpp"

#include <tbd/websocket_api/websocket_server.hpp>

namespace esphome::tbd_websocket_api {

WebsocketServer::WebsocketServer() {

}

void WebsocketServer::setup() {
    tbd::websocket_api::begin();
}

float WebsocketServer::get_setup_priority() const {
    return setup_priority::AFTER_WIFI;
}

void WebsocketServer::on_shutdown() {
    tbd::websocket_api::end();
}

}
