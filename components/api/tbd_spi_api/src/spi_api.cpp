#include <tbd/spi_api/spi_api.hpp>


namespace tbd::spi_api {

[[tbd::sink]]
void emit_on_websocket(const uint8_t* buffer, size_t length) {
//    httpd_ws_frame_t ws_pkt = {};
//    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
//    ws_pkt.payload = const_cast<uint8_t*>(buffer);
//    ws_pkt.len = length;
//    if (const auto err = broadcast_send(&ws_pkt); err != ESP_OK) {
//        ESP_LOGE(tag, "failed to send event");
//    }
}

}