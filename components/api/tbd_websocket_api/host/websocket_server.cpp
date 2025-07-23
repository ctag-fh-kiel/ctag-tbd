#include <tbd/websocket_api/websocket_server.hpp>
#include <tbd/websocket_api/module.hpp>

#include <tbd/api.hpp>

#include <tbd/logging.hpp>
#include <server_ws.hpp>

#include <memory>


namespace {

struct WEBSOCKET_SERVER {};

using tbd::websocket_api::tag;
using WSServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

WSServer server;
std::thread server_thread;
std::promise<unsigned short> server_port;

}

namespace tbd::websocket_api {

void begin() {
    TBD_LOGI(tag, "starting websocket server");
    
    server.config.port = TBD_WEBSOCKET_PORT;
    auto &echo = server.endpoint["^/ws/?$"];
    echo.on_message = [](std::shared_ptr<WSServer::Connection> connection, std::shared_ptr<WSServer::InMessage> request) {
        const auto in_data = request->string();
        tbd::api::ApiPacketHandler<WEBSOCKET_SERVER, true> packet_handler;
            const auto response_length = packet_handler.handle(in_data);
        if (response_length == 0) {
            return;
        }

        connection->send(packet_handler.buffer_view(), nullptr, 130);

      };

    echo.on_open = [](std::shared_ptr<WSServer::Connection> connection) {
        TBD_LOGI(tag, "opened connection %p", connection.get());
    };

    echo.on_close = [](std::shared_ptr<WSServer::Connection> connection, int status, const std::string& reason) {
        TBD_LOGI(tag, "closed connection %p, %i, %s", connection.get(), status, reason.c_str());
    };

    echo.on_error = [](std::shared_ptr<WSServer::Connection> connection, const SimpleWeb::error_code &ec) {
        TBD_LOGE(tag, "connection error: %p, %s", connection.get(), ec.message().c_str());
    };

    server_thread = std::thread([]() {
        server.start([](unsigned short port) {
          server_port.set_value(TBD_WEBSOCKET_PORT);
        });
    });
    
}
    
void end() {
    server.stop();
}

[[tbd::sink]]
void emit_on_websocket(const uint8_t* buffer, size_t length) {
    const std::string_view buffer_view(reinterpret_cast<const char*>(buffer), length);
    for (auto& connection : server.get_connections()) {
        connection->send(buffer_view, nullptr, 130);
    }
}

[[tbd::sink]]
void foo_sink(const uint8_t* buffer, size_t length) {

}

[[tbd::sink]]
void bar_sink(const uint8_t* buffer, size_t length) {

}

}