#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd::serialization {

enum WireType {
    VARIANT           = 0,
    I64               = 1,
    LEN               = 2,
    SGROUP            = 3,
    EGROUP            = 4,
    I32               = 5,
    INVALID_WIRE_TYPE = 6
};

inline bool is_final_byte(const uint8_t byte) {
    return byte & 0b10000000 == 0;
}

inline uint8_t get_wire_type(const uint8_t tag) {
    return tag & 0b000001111;
}

inline uint8_t get_field_number(const uint8_t tag) {
    return (tag & 0b11111000) >> 3;
}

inline write_par(uint8_t field_number, const int32_t value, uint8_t* buffer) {
    buffer[0] = field_number << 3 | VARIANT;
}

inline write_par(uint8_t field_number, const uint32_t value, uint8_t* buffer) {
    buffer[0] = field_number << 3 | VARIANT;
}

}