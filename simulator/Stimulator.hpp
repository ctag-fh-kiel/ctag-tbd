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

#include "helpers/ctagSineSource.hpp"

class Stimulator {
public:
    Stimulator();

    ~Stimulator();

    float Process();

    void SetMode(const int m);

    void SetValue(const int v);

private:
    CTAG::SP::HELPERS::ctagSineSource src;
    enum ModeType {
        MANUAL = 0, PULSE = 1, USINE = 2, BISINE = 3, BISTEPS = 4
    };
    ModeType mode;
    float value;
    float f;
    const float fs = 44100.f / 32.f;
};
