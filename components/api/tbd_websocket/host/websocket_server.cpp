#include <tbd/api/websocket_server.hpp>
#include <tbd/websocket/module.hpp>

#include <tbd/api.hpp>

#include <tbd/logging.hpp>
#include <server_ws.hpp>

#include <memory>

namespace {

using tbd::websocket::tag;
using WSServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

WSServer server;
std::thread server_thread;
std::promise<unsigned short> server_port;

}

namespace tbd::api {

void WebsocketServer::begin() {
    TBD_LOGI(tag, "starting websocket server");
    
    server.config.port = TBD_WEBSOCKET_PORT;
    auto &echo = server.endpoint["^/ws/?$"];
    echo.on_message = [](std::shared_ptr<WSServer::Connection> connection, std::shared_ptr<WSServer::InMessage> request) {
        std::string in_message = request->string();
        size_t in_length = in_message.length();
        size_t length = in_length > 128 ? in_length : 128;
        uint8_t buffer[length];
        for (size_t i = 0; i < in_message.length(); ++i) {
          buffer[i] = in_message[i];
        }
        if (Api::handle_stream_input(buffer, length) != errors::SUCCESS) {
          return;
        }
        auto out_message = std::string_view(reinterpret_cast<char*>(buffer), length);
        connection->send(out_message, nullptr, 130);
      };
    

    server_thread = std::thread([]() {
        server.start([](unsigned short port) {
          server_port.set_value(TBD_WEBSOCKET_PORT);
        });
    });
    
}
    
void WebsocketServer::end() {

}

}