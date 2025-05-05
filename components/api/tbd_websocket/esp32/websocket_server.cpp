#include <tbd/api/websocket_server.hpp>
#include <tbd/websocket/module.hpp>

#include <tbd/logging.hpp>

#include <esp_http_server.h>

using tbd::websocket::tag;

namespace {

httpd_handle_t server = nullptr;

}

namespace tbd::api {

void WebsocketServer::begin() {
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    TBD_LOGI(tag, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        TBD_LOGI(tag, "Registering URI handlers");
        // httpd_register_uri_handler(server, &ws);
    }

    TBD_LOGI(tag, "Error starting server!");
}
    
void WebsocketServer::end() {
    auto err = httpd_stop(server);
}

}