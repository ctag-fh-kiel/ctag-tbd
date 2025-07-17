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
#include <tbd/sound_utils/filters/ctagDiodeLadderFilter2.hpp>
#include <tbd/sound_utils/ctagFastMath.hpp>

void tbd::sound_utils::ctagDiodeLadderFilter2::Init() {
    z1 = 0.0f;
    z2 = 0.0f;
    z3 = 0.0f;
    z4 = 0.0f;
    q = -1.0f;
    k = 0.0f;
    g = 0.0f;
    G = 0.0f;
    G2 = 0.0f;
    G3 = 0.0f;
    GAMMA = 0.0f;

    SetResonance(1.f);
    SetCutoff(1000.f);
}

float tbd::sound_utils::ctagDiodeLadderFilter2::Process(float in) {
    float g_plus_1 = g + 1.0f;

    float S1 = z1 / g_plus_1;
    float S2 = z2 / g_plus_1;
    float S3 = z3 / g_plus_1;
    float S4 = z4 / g_plus_1;

    float S = (G3 * S1) + (G2 * S2) + (G * S3) + S4;
    float u = (in - k * S) / (1.f + k * GAMMA);

    // 1st stage
    float v = (u - z1) * G;
    float lp = v + z1;
    z1 = lp + v;

    // 2nd stage
    v = (lp - z2) * G;
    lp = v + z2;
    z2 = lp + v;

    // 3rd stage
    v = (lp - z3) * G;
    lp = v + z3;
    z3 = lp + v;

    // 4th stage
    v = (lp - z4) * G;
    lp = v + z4;
    z4 = lp + v;

    return lp;
}

void tbd::sound_utils::ctagDiodeLadderFilter2::SetCutoff(float cutoff) {
    cutoff_ = cutoff;
    wd = 2.f * M_PI * cutoff;
    wa = two_div_T * fasttan(wd * Tdiv2);
    g = wa * Tdiv2;
    G = g / (1.0f + g);
    G2 = G * G;
    G3 = G2 * G;
    GAMMA = G2 * G2;
}

// 0..1 for resonance
void tbd::sound_utils::ctagDiodeLadderFilter2::SetResonance(float resonance) {
    resonance *= 24.5f;
    resonance += 0.5f;
    q = (resonance < 0.5f) ? 0.5f : (resonance > 25.0f) ? 25.0f : resonance;
    k = (4.0f * (q - 0.5f)) / (25.0f - 0.5f);
}

void tbd::sound_utils::ctagDiodeLadderFilter2::SetSampleRate(float fs) {
    fs_ = fs;
    T = 1.f / fs;
    Tdiv2 = T / 2.0f;
    two_div_T = 2.0f / T;
}
