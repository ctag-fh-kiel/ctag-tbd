#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/errors.hpp>

namespace tbd::websocket_api {

void begin();
void end();

Error emit_event(const api::Packet& event);

}
