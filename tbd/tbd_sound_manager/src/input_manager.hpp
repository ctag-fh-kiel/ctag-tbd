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
#include "esp_attr.h"

namespace tbd::audio {

class InputManager final{
public:
    InputManager() = delete;
    static void Init();
    static void FlushBuffers();
    static void SetCVChannelBiPolar(bool const &v0, bool const &v1, bool const &v2, bool const &v3);
    IRAM_ATTR static void Update(uint8_t **trigs, float **cvs);
private:
#ifndef CONFIG_TBD_PLATFORM_MK2
    DRAM_ATTR static uint8_t trig_data[N_TRIGS];
    DRAM_ATTR static float cv_data[N_CVS];
#endif
};

}
