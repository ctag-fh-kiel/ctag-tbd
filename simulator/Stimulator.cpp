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

#include "Stimulator.hpp"
#include <cmath>

Stimulator::Stimulator() {
    src.SetSampleRate(fs);
    src.SetFrequency(1.f);
    mode = ModeType::MANUAL;
    value = 1.f;
}

Stimulator::~Stimulator() {

}

void Stimulator::SetMode(const int m) {
    mode = (ModeType) m;
}

void Stimulator::SetValue(const int v) {
    value = (float) v / 4095.f;
    f = value * 1000.f;
    src.SetFrequency(f);
}

float Stimulator::Process() {
    switch (mode) {
        case ModeType::MANUAL:
            src.Process();
            return value;
        case ModeType::BISINE:
            return src.Process();
        case ModeType::USINE:
            return (src.Process() + 1.f) / 2.f;
        case ModeType::PULSE:
            return (src.Process() >= 0.f ? 1.f : 0.f);
        case ModeType::BISTEPS:
            return 0.f; // not implemented
        default:
            return 0.f;
    }
}
