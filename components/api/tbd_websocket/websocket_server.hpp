#pragma once

#include <esphome/core/component.h>
#include <tbd/api/websocket_server.hpp>

namespace esphome::tbd_websocket {

class WebsocketServer  : public Component {
public:
  WebsocketServer();
  void setup() override;
//  uint16_t get_port() const;
  float get_setup_priority() const override;
//  void loop() override;
//  void dump_config() override;
  void on_shutdown() override;
};

}
