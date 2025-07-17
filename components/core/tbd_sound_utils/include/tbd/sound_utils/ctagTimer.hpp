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

#include <functional>

namespace tbd::sound_utils {
    
class ctagTimer {
    float current_time = 0.f;
    float delta_time = 1.f / 44100.f;
    float timeout = 1000.f;
    std::function<void()> timeout_cb = nullptr;
    bool expired = true;
    bool repeat = false;
public:
    void SetSampleRate(float fs_Hz);
    void SetTimeout(float t_ms);
    void Tick();
    void SetRepeat(bool rpt);
    bool isRepeat();
    bool isExpired();
    void SetTimeoutCallback(std::function<void()> cb);
};

}

