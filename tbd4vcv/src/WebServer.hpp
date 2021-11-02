#pragma once
#include <thread>
#include <iostream>
#include <libusockets.h>
#include <string>
#include "AsyncFileReader.h"
#include "AsyncFileStreamer.h"
#include "Middleware.h"
#include "App.h"

class WebServer {
public:
    void Start();
    ~WebServer();
private:
    std::thread server_thread;
    us_listen_socket_t *token;
    AsyncFileStreamer asyncFileStreamer(std::string("www/"));
};

