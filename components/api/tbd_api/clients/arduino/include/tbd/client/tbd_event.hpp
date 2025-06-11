#pragma once

#include <tbd/api/packet.hpp>


namespace tbd::client {

struct Event {
    explicit Event(ClientBase& sender) : sender_(sender) {}

    void handle_event(const api::Packet& event) {
        TBD_LOGI(tag, "got event %i", event.handler);
    }

private:
    ClientBase& sender_;
};

}
