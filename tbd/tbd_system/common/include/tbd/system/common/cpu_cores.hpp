#pragma once

#include <cinttypes>


namespace tbd::system {

enum class CpuCore: uint8_t {
    system = 0,
    audio = 1,
};

}
