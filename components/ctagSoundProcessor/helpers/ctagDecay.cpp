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


//
// Created by Robert Manzke on 06.03.20.
//

#include "ctagDecay.hpp"
#include "helpers/ctagFastMath.hpp"
#include "esp_log.h"
#include "math.h"


CTAG::SP::HELPERS::ctagDecay::ctagDecay() :
        _c(0.f), out(0.f), _fs(44100.f), pre_c(0.f), smooth_c(true), smooth_level(1.f / 32.f) {

}

void CTAG::SP::HELPERS::ctagDecay::SetDecay60dB(float val) {
    const float LOG001 = -3.f;
    SetCoeff(fastexp(LOG001 / (val * _fs))); // -60dB, accuracy with large vals is bad
    //ESP_LOGE("calc", "val %.6f, _c %.6f", val, _c);
}

float CTAG::SP::HELPERS::ctagDecay::Process(float in) {
    if (smooth_c) {
        pre_c = _c * smooth_level + (1.f - smooth_level) * pre_c;
    } else {
        pre_c = _c;
    }
    out = in + pre_c * out;
    return out;
}

void CTAG::SP::HELPERS::ctagDecay::SetSampleRate(float fs) {
    _fs = fs;
}

void CTAG::SP::HELPERS::ctagDecay::SetCoeff(float c) {
    _c = c;
}

float CTAG::SP::HELPERS::ctagDecay::GetCoeff() {
    return _c;
}

void CTAG::SP::HELPERS::ctagDecay::SetCoeffSmoothing(bool yes) {
    smooth_c = yes;
}

void CTAG::SP::HELPERS::ctagDecay::SetCoeffSmoothingLevel(float lev) {
    smooth_level = lev;
}
