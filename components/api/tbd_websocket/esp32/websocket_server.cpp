#include <tbd/api/websocket_server.hpp>
#include <tbd/websocket/module.hpp>

#include <tbd/logging.hpp>

#include <esp_http_server.h>

using tbd::websocket::tag;

namespace {

httpd_handle_t server = nullptr;

esp_err_t websocket_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        TBD_LOGI(tag, "new client connected");
        return 0;
    }

    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        TBD_LOGE(tag, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    TBD_LOGI(tag, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        size_t length = ws_pkt.len + 1 ;
        length = length < 128 ? 128 : length;
        uint8_t packet_buffer[length];
        ws_pkt.payload = packet_buffer;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            TBD_LOGE(tag, "httpd_ws_recv_frame failed with %d", ret);
            return ret;
        }
        TBD_LOGI(tag, "Got packet with message: %s", ws_pkt.payload);
    }
    // TBD_LOGI(tag, "Packet type: %d", ws_pkt.type);
    // if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
    //     strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
    //     free(buf);
    //     return trigger_async_send(req->handle, req);
    // }

    // ret = httpd_ws_send_frame(req, &ws_pkt);
    // if (ret != ESP_OK) {
    //     TBD_LOGE(tag, "httpd_ws_send_frame failed with %d", ret);
    // }
    return ret;
}

const httpd_uri_t websocket_endpoint = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = websocket_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};

}

namespace tbd::api {

void WebsocketServer::begin() {
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    TBD_LOGI(tag, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        TBD_LOGI(tag, "Registering URI handlers");
        httpd_register_uri_handler(server, &websocket_endpoint);
    }

    TBD_LOGI(tag, "Error starting server!");
}
    
void WebsocketServer::end() {
    auto err = httpd_stop(server);
}

}