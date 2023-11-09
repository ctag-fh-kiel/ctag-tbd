#pragma once

#include <cstdint>
#include "SPManagerDataModel.hpp"
#include "esp_attr.h"

namespace CTAG{
    namespace CTRL{
        class Control final{
        public:
            Control() = delete;
            static void Init();
            static void SetCVChannelBiPolar(bool const &v0, bool const &v1, bool const &v2, bool const &v3);
            IRAM_ATTR static void Update(uint8_t **trigs, float **cvs);
        private:
#ifndef CONFIG_TBD_PLATFORM_MK2
            DRAM_ATTR static uint8_t trig_data[N_TRIGS];
            DRAM_ATTR static float cv_data[N_CVS];
#endif
        };
    }
}

