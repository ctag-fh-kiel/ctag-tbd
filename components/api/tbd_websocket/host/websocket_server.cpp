#include <tbd/api/websocket_server.hpp>
#include <tbd/websocket/module.hpp>

#include <tbd/api/packet_parser.hpp>
#include <tbd/api/packet_writers.hpp>
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

namespace tbd::api::websocket_server {

void begin() {
    TBD_LOGI(tag, "starting websocket server");
    
    server.config.port = TBD_WEBSOCKET_PORT;
    auto &echo = server.endpoint["^/ws/?$"];
    echo.on_message = [](std::shared_ptr<WSServer::Connection> connection, std::shared_ptr<WSServer::InMessage> request) {
        auto in_data = request->string();
        PacketParser parser;
        if (!parser.parse_from_string(in_data)) {
            return;
        }

        PacketBufferWriter writer;
        Packet response;
        if (Api::handle_rpc(parser.packet(), response, writer.payload_buffer(), writer.payload_buffer_size()) != errors::SUCCESS) {
          return;
        }
        writer.write(response);
        connection->send(writer.buffer_view(), nullptr, 130);
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

}

}