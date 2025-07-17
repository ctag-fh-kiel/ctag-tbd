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

#include <cmath>
#include <cstdint>


namespace tbd::sound_utils {

class ctagWNoiseGen {
public:
    ctagWNoiseGen() {

    }

    ctagWNoiseGen(int32_t seed) {
        sd = seed;
    }

    void SetBipolar(bool yes) {
        isBipolar = yes;
    }

    void ReSeed(int32_t seed) {
        sd = seed;
    }

    float Process() {
// from http://www.musicdsp.org/archive.php?classid=5#273
        sd *= 16807;
        if (isBipolar)return (float) sd * 4.6566129e-010f;
        return (float) (sd & 0x7FFFFFFF) * 4.6566129e-010f;
    }

private:
    int32_t sd = 42;
    bool isBipolar = true;
};

}

