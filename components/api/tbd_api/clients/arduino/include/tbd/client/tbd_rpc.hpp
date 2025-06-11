#pragma once

#include <tbd/client/module.hpp>
#include <tbd/logging.hpp>

#include <tbd/api/packet.hpp>


namespace tbd::client {

struct RPC {
    explicit RPC(ClientBase& sender): sender_(sender) {}

private:
    ClientBase& sender_;
};

}
