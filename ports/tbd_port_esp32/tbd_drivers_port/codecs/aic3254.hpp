#ifndef MAIN_AIC3254_HPP
#define MAIN_AIC3254_HPP

#include <cstdint>
#include "driver/gpio.h"

class aic3254 final {
    gpio_num_t _pinsda, _pinscl, _pinreset;
    uint32_t _i2cspeed;
    void write_reg(uint8_t reg_add, uint8_t data);
    void write_AIC32X4_reg(uint8_t reg_add, uint8_t data);
    uint8_t read_16bit_reg(uint8_t reg_add);
    uint8_t read_reg(uint8_t reg_add);
    bool test_reg(uint8_t register_address, uint8_t expected_value);
    uint8_t page {255};
    // lookup table for volume mapping, 58 corresponds to 0x00, values from 0 to 57 have lsb set to 1 and start from 0x81 to 0xFF, 59 to 63 are from 0x01 to 0x30
    const uint8_t volume_mapping[64] {
        0x81, 0x83, 0x85, 0x87, 0x89, 0x8B, 0x8D, 0x8F, 0x91, 0x93, 0x95, 0x97, 0x99, 0x9B, 0x9D, 0x9F,
        0xA1, 0xA3, 0xA5, 0xA7, 0xA9, 0xAB, 0xAD, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7, 0xB9, 0xBB, 0xBD, 0xBF,
        0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF, 0xD1, 0xD3, 0xD5, 0xD7, 0xD9, 0xDB, 0xDD, 0xDF,
        0xE1, 0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xED, 0xEF, 0xF5, 0xFC, 0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D};
public:
    aic3254();
    ~aic3254();
    void init();
    bool identify();
    bool setOutputVolume(uint8_t lvol, uint8_t rvol);
};

#endif //MAIN_AIC3254_HPP
