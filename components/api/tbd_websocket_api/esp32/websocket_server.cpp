#include <tbd/websocket_api/websocket_server.hpp>
#include <tbd/websocket_api/module.hpp>

#include <tbd/logging.hpp>
#include <tbd/api.hpp>

#include <esp_http_server.h>

using tbd::websocket_api::tag;

namespace {

httpd_handle_t server = nullptr;

struct WEBSOCKET_SERVER {};

// IDF httpd is single threaded, use fixed buffers
tbd::api::Packet::PacketBuffer input_buffer;

esp_err_t broadcast_send(httpd_ws_frame_t *ws_pkt) {
    size_t num_active_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    int client_fds[CONFIG_LWIP_MAX_LISTENING_TCP] = {0};

    if (const auto ret = httpd_get_client_list(server, &num_active_clients, client_fds); ret != ESP_OK) {
        TBD_LOGE(tag, "failed to retrieve list of connected clients");
        return ret;
    }

    for (int i = 0; i < num_active_clients; i++) {
        if (const auto client_info = httpd_ws_get_fd_info(server, client_fds[i]);
            client_info == HTTPD_WS_CLIENT_WEBSOCKET)
        {
            httpd_ws_send_frame_async(server, client_fds[i], ws_pkt);
        }
    }

    return ESP_OK;
}

esp_err_t websocket_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        TBD_LOGI(tag, "new client connected");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;

    esp_err_t err;
    if (err = httpd_ws_recv_frame(req, &ws_pkt, 0); err != ESP_OK) {
        TBD_LOGE(tag, "httpd_ws_recv_frame failed to get frame len with %d", err);
        return err;
    }
    TBD_LOGI(tag, "frame len is %d", ws_pkt.len);

    if (!ws_pkt.len) {
        TBD_LOGE(tag, "failed to determine package length");
        return ESP_ERR_INVALID_ARG;
    }

    ws_pkt.payload = input_buffer;
    if (err = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len); err != ESP_OK) {
        TBD_LOGE(tag, "httpd_ws_recv_frame failed with %d", err);
        return err;
    }

    tbd::api::ApiPacketHandler<WEBSOCKET_SERVER, false> packet_handler;
    const auto response_length = packet_handler.handle(input_buffer, ws_pkt.len);
    if (response_length == 0) {
        return ESP_OK;
    }

    ws_pkt.payload = const_cast<uint8_t*>(packet_handler.output_buffer());
    ws_pkt.len = response_length;

    TBD_LOGI(tag, "Sending packet with message: %i", ws_pkt.len);
    if (err = httpd_ws_send_frame(req, &ws_pkt); err != ESP_OK) {
        TBD_LOGE(tag, "httpd_ws_send_frame failed with %d", err);
        return err;
    }
    return ESP_OK;
}

const httpd_uri_t websocket_endpoint = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = websocket_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};

}

namespace tbd::websocket_api {

void begin() {
    TBD_LOGI(tag, "Starting server");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    TBD_LOGI(tag, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        TBD_LOGI(tag, "Registering URI handlers");
        httpd_register_uri_handler(server, &websocket_endpoint);
    }

    TBD_LOGI(tag, "listening on port :%i", config.server_port);
}
    
void end() {
    auto err = httpd_stop(server);
}

[[tbd::sink]]
void emit_on_websocket(const uint8_t* buffer, size_t length) {
    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
    ws_pkt.payload = const_cast<uint8_t*>(buffer);
    ws_pkt.len = length;
    if (const auto err = broadcast_send(&ws_pkt); err != ESP_OK) {
        ESP_LOGE(tag, "failed to send event");
    }
}

}