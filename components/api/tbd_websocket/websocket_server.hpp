#pragma once

#include <esphome/core/component.h>
#include <tbd/api/websocket_server.hpp>

namespace esphome::tbd_websocket {

class WebsocketServer  : public Component {
public:
  WebsocketServer();
  void setup() override;
  float get_setup_priority() const override;
  void on_shutdown() override;
};

}
