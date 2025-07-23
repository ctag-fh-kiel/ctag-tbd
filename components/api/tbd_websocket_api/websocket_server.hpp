#pragma once

#include <esphome/core/component.h>

namespace esphome::tbd_websocket_api {

class WebsocketServer  : public Component {
public:
  WebsocketServer();
  void setup() override;
  float get_setup_priority() const override;
  void on_shutdown() override;
};

}
