#pragma once

#include <esphome/core/component.h>

#include <tbd/spi_api/spi_api.hpp>


namespace esphome::tbd_spi_api {

/** esphome based uart API server
 *
 *  NOTE: The underlying implementation for esp32 uses locking that can not be disabled.
 */
class SPIServer : public Component {
public:
    SPIServer() {}

    void setup() override {
        tbd::spi_api::begin();
    }

    void on_shutdown() override {
        tbd::spi_api::end();
    }

    void loop() override {
        tbd::spi_api::do_work();
    }
};

}
