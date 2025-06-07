#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/errors.hpp>

namespace tbd::api::websocket_server {

void begin();
void end();

Error emit_event(const Packet& event);

}
