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

#include "dsps_biquad.h"

namespace CTAG::SP::HELPERS {
    enum class BIQUAD_TYPE : uint32_t {
        LP, BP, HP
    };

    class ctagBiQuad {
    public:
        ctagBiQuad();

        void SetType(BIQUAD_TYPE t);

        void Mute();

        //void Process(float *data, const uint32_t sz, const uint32_t inc);
        //float Process(float data);
        void Process(float *data, const uint32_t sz);

        void SetCutoffHz(float cut);

        void SetQ(float q);

        void SetSampleRate(float fs);

    private:
        BIQUAD_TYPE type;

        void calcCoeffs();

        float w[2];
        float coeff[6];
        float _cut, _q;
        float _fs;
    };
}


