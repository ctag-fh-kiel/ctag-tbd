/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2024 by Robert Manzke. All rights reserved.

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
#include "stmlib/dsp/filter.h"
#include <tbd/sound_utils/ctagADEnv.hpp>
#include <tbd/sound_utils/ctagTimer.hpp>

namespace tbd::sound_utils::synthesis {

class Clap {
public:
    void Init();
    void Trigger();
    void Process(float* out, uint32_t size);
    struct Params{
        float pitch1, pitch2; // 350/44100.f - 4000/44100.f, 300/44100.f - 3000/44100.f
        float reso1, reso2; // 1 - 2.5, 0.75 - 6.5
        float decay1, decay2; // 0.05 - 0.3, 0.05 - 2
        float attack; // 0 - 0.1
        float scale; // 1 - 3
        uint32_t transient; // 0..15
    };

    Params params;
private:
    stmlib::Svf svf1, svf2;
    tbd::sound_utils::ctagADEnv env1;
    tbd::sound_utils::ctagADEnv env2;
    tbd::sound_utils::ctagTimer timers[4];

    float a1 {1.f}, a2 {1.f};

    // transients
    const float delays[16][4] {
        {10.f, 20.f, 30.f, 40.f},
        {20.f, 30.f, 40.f, 60.f},
        {20.f, 40.f, 60.f, 80.f},
        {10.f, 25.f, 40.f, 60.f},
        {30.f, 45.f, 60.f, 90.f},
        {10.f, 20.f, 35.f, 60.f},
        {15.f, 30.f, 50.f, 65.f},
        {10.f, 20.f, 40.f, 80.f},
        {10.f, 20.f, 60.f, 70.f},
        {15.f, 30.f, 45.f, 60.f},
        {40.f, 80.f, 90.f, 110.f},
        {30.f, 40.f, 70.f, 75.f},
        {5.f, 30.f, 40.f, 50.f},
        {15.f, 45.f, 55.f, 65.f},
        {15.f, 20.f, 35.f, 40.f},
        {10.f, 40.f, 50.f, 70.f}
    };

    const float amplitudes[16][4] {
        {1.f, 1.f, 1.f, 0.1f},
        {1.f, .5f, 1.f, 0.1f},
        {1.f, .5f, 1.f, 0.1f},
        {.5f, 1.f, 1.f, 0.1f},
        {1.f, .1f, 1.f, 0.1f},
        {1.f, .5f, .5f, 0.2f},
        {1.f, .2f, .2f, 0.5f},
        {.25f, .5f, 1.f, 0.2f},
        {.25f, 1.f, .25f, 0.5f},
        {1.f, .5f, .25f, 0.5f},
        {1.f, .2f, 1.f, 0.2f},
        {0.25f, .5f, .25f, 1.f},
        {.5f, 1.f, 0.25f, 0.1f},
        {.5f, .1f, 1.f, 0.25f},
        {1.f, .1f, 1.f, 0.2f},
        {1.f, .5f, 1.f, 0.2f}
    };
};

}


