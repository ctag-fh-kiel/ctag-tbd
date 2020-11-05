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
#include "ctagWNoiseGen.hpp"


namespace CTAG::SP::HELPERS {
    class ctagGNoiseGen {
    public:
        ctagGNoiseGen() :
                wnoise(815) {
            SetPrecision(32);
        }

        void SetPrecision(int32_t prec) {
            int32_t q = prec;
            float c1 = (1 << q) - 1;
            gwn3_c2 = ((int32_t) (c1 / 3)) + 1;
            gwn3_c3 = 1. / c1;
        }

        float Process() {
            // http://www.musicdsp.org/archive.php?classid=0#129
            // http://www.musicdsp.org/archive.php?classid=1#168
            // This algorithm (adapted from "Natur als fraktale Grafik" by Reinhard Scholl) implements a generation method for gaussian
            // distributed random numbers with mean=0 and variance=1 (standard gaussian distribution) mapped to the range of
            // -1 to +1 with the maximum at 0.
            // For only positive results you might abs() the return value. The q variable defines the precision, with q=15 the smallest
            // distance between two numbers will be 1/(2^q div 3)=1/10922 which usually gives good results.
            // Note: the random() function used is the standard random function from Delphi/Pascal that produces *linear*
            // distributed numbers from 0 to parameter-1, the equivalent C function is probably rand().
            float random = wnoise.Process();
            return (2. * ((random * gwn3_c2) + (random * gwn3_c2) + (random * gwn3_c2)) - 3. * (gwn3_c2 - 1.)) *
                   gwn3_c3;
        }

    private:
        ctagWNoiseGen wnoise;
        float gwn3_c2, gwn3_c3;
    };
}

