#include "WebServer.hpp"
#include "App.h"

WebServer::~WebServer() {
    std::cerr << "Web server destructor" << std::endl;
    us_listen_socket_close(0, token);
    server_thread.join();
}

void WebServer::Start(){
    std::thread st( [this] () {
        /* Overly simple hello world app */
        uWS::App().get("/*", [this](auto *res, auto */*req*/) {
            res->end("Hello world!");
        }).listen(3000, [this](auto *listen_socket) {
            if (listen_socket) {
                token = listen_socket;
                std::cout << "Listening on port " << 3000 << std::endl;
            }
        }).run();

        std::cout << "Failed to listen on port 3000" << std::endl;
    });
    server_thread = std::move(st);
}