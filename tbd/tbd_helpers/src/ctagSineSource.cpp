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

#include <cmath>
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagFastMath.hpp"

CTAG::SP::HELPERS::ctagSineSource::ctagSineSource() {
    fSample = 44100.f;
    SetFrequencyPhase(1.f, 0.f);
}

void CTAG::SP::HELPERS::ctagSineSource::SetSampleRate(float f_hz) {
    fSample = f_hz;
}

void CTAG::SP::HELPERS::ctagSineSource::SetFrequency(float f_hz) {
    a = 2.f * (float) fastsin(M_PI * f_hz / fSample);
}

float CTAG::SP::HELPERS::ctagSineSource::Process() {
    s[0] = s[0] - a * s[1]; // sin
    s[1] = s[1] + a * s[0]; // cos
    return s[0];
}

float CTAG::SP::HELPERS::ctagSineSource::GetCos() {
    return s[1];
}

float CTAG::SP::HELPERS::ctagSineSource::GetSin() {
    return s[0];
}

void CTAG::SP::HELPERS::ctagSineSource::SetFrequencyPhase(float f_hz, float phase_rad) {
    a = 2.f * (float) fastsin(M_PI * f_hz / fSample);
    s[0] = 0.5f * fastcos(phase_rad);
    s[1] = 0.5f * fastsin(phase_rad);
}
