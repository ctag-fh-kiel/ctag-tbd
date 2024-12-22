#pragma once

#include <tbd/system/common/random_pre.hpp>

#if defined(__xtensa__)
    #include "xtensa/core-macros.h"

    namespace tbd::system {

    inline uint32_t get_seed() { return XTHAL_GET_CCOUNT(); }

    }
#elif defined(__riscv)
    #include <esp_random.h>

    namespace tbd::system {

    inline uint32_t get_seed() { return esp_random(); }

    }
#else
    #error "the current esp32 architecture is not supported"
#endif
