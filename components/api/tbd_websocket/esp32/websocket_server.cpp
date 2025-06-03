#include <tbd/api/websocket_server.hpp>
#include <tbd/websocket/module.hpp>

#include <tbd/logging.hpp>
#include <tbd/api.hpp>
#include <tbd/api/packet_parser.hpp>
#include <tbd/api/packet_writers.hpp>

#include <esp_http_server.h>

using tbd::websocket::tag;

namespace {

httpd_handle_t server = nullptr;

// esp httpd is single threaded, so no need to risk stack overflows
tbd::api::Packet::PacketBuffer input_buffer;
tbd::api::PacketBufferWriter writer;


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

//    size_t length = ws_pkt.len + 1; // just to ensure bad string payloads do not result in bad outcomes add 0
//    size_t buffer_size = length > 128 ? length : 128;
//    uint8_t packet_buffer[buffer_size + 1];
    ws_pkt.payload = input_buffer;

    if (err = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len); err != ESP_OK) {
        TBD_LOGE(tag, "httpd_ws_recv_frame failed with %d", err);
        return err;
    }

    tbd::api::PacketParser parser;
    if (!parser.parse_from_buffer(input_buffer, tbd::api::Packet::BUFFER_SIZE)) {
        return ESP_FAIL;
    }

    tbd::api::Packet response;
    if (tbd::Api::handle_rpc(parser.packet(), response, writer.payload_buffer(), writer.payload_buffer_size()) != tbd::errors::SUCCESS) {
        return ESP_FAIL;
    }
    writer.write(response);

    ws_pkt.payload = const_cast<uint8_t*>(writer.buffer());
    ws_pkt.len = writer.serialized_length();
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

namespace tbd::api::websocket_server {

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

}