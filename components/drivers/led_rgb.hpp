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

#include "driver/ledc.h"

namespace CTAG{
    namespace DRIVERS{
        class LedRGB{
            public:
            // rgb are each 8 bit valued PWM
                static void InitLedRGB();
                static void GetLedRGB(int &r, int &g, int &b);
                static void SetLedRGB(int r, int g, int b);
                static void SetLedR(int r);
                static void SetLedG(int g);
                static void SetLedB(int b);
            private:
                static ledc_channel_config_t ledc_channel[3];
        };
    }
}

