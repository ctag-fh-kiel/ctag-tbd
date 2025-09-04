#pragma once
#include <cstdint>

namespace tbd::spi_inputs::tusb {

static void begin();
static uint32_t read(uint8_t *data, uint32_t len);
static uint32_t write(uint8_t const *data, uint32_t len);

}
