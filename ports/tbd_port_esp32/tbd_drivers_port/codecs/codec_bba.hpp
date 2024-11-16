 /***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#pragma once

#include <cstdint>
#include "sdkconfig.h"

#ifdef CONFIG_TBD_BBA_CODEC_ES8388
#include "es8388.hpp"
#else
#include "aic3254.hpp"
#endif

namespace CTAG {
    namespace DRIVERS {

        class Codec {
        public:
            Codec() = delete;
            static void InitCodec();

            static void HighPassEnable();

            static void HighPassDisable();

            static void RecalibDCOffset();

            static void SetOutputLevels(const uint32_t left, const uint32_t right);

            static void ReadBuffer(float *buf, uint32_t sz);

            static void WriteBuffer(float *buf, uint32_t sz);

        private:
#ifdef CONFIG_TBD_BBA_CODEC_ES8388
            static es8388 codec;
#else
            static aic3254 codec;
#endif

        };
    }
}
