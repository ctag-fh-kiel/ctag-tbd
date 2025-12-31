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
#include "ctagDiodeLadderFilter.hpp"
#include "../helpers/ctagFastMath.hpp"

void CTAG::SP::HELPERS::ctagDiodeLadderFilter::Init() {
    int i;

    a[0] = 1.0f;
    a[1] = 0.5f;
    a[2] = 0.5f;
    a[3] = 0.5f;

    for (i = 0; i < 4; i++) {
        z[i] = 0.0f;
        G[i] = 0.0f;
        beta[i] = 0.0f;
        SG[i] = 0.0f;
    }

    for (i = 0; i < 3; i++) {
        delta[i] = 0.0f;
        epsilon[i] = 0.0f;
        gamma[i] = 0.0f;
    }
    GAMMA = 0.0f;
    SIGMA = 0.0f;

    SetResonance(1.f);
    SetCutoff(1000.f);
}

float CTAG::SP::HELPERS::ctagDiodeLadderFilter::Process(float in) {
    // All coefficient calculations moved to SetCutoff/updateCoefficients
    // This eliminates ~40+ operations per sample!

    // Feedback inputs - compute intermediate values to avoid duplication
    float fb4 = beta[3] * z[3];
    float fb4_delta2 = fb4 * delta[2];
    float temp2 = z[2] + fb4_delta2;
    float fb3 = beta[2] * temp2;

    float fb3_delta1 = fb3 * delta[1];
    float temp1 = z[1] + fb3_delta1;
    float fb2 = beta[1] * temp1;

    float fb2_delta0 = fb2 * delta[0];
    float temp0 = z[0] + fb2_delta0;

    // Feedback process - reuse temp values instead of recalculating
    float fbo1 = beta[0] * temp0;
    float fbo2 = beta[1] * temp1;  // Reuse temp1 (was duplicate calculation)
    float fbo3 = beta[2] * temp2;  // Reuse temp2 (was duplicate calculation)

    SIGMA = (SG[0] * fbo1) + (SG[1] * fbo2) + (SG[2] * fbo3) + (SG[3] * fb4);

    // form input to loop - use pre-calculated reciprocal (multiply is 4-10x faster than divide)
    float un = (in - kval_ * SIGMA) * reciprocal_kval_GAMMA;

    // 1st stage
    float xin = un * gamma[0] + fb2 + epsilon[0] * fbo1;
    float v = (a[0] * xin - z[0]) * alpha;
    float lp = v + z[0];
    z[0] = lp + v;

    // 2nd stage - use pre-calculated alpha_half to avoid 0.5f multiplication
    xin = lp * gamma[1] + fb3 + epsilon[1] * fbo2;
    v = (xin - z[1] - z[1]) * alpha_half;  // (0.5f * xin - z[1]) * alpha optimized
    lp = v + z[1];
    z[1] = lp + v;

    // 3rd stage - use pre-calculated alpha_half
    xin = lp * gamma[2] + fb4 + epsilon[2] * fbo3;
    v = (xin - z[2] - z[2]) * alpha_half;
    lp = v + z[2];
    z[2] = lp + v;

    // 4th stage - use pre-calculated alpha_half
    v = (lp - z[3] - z[3]) * alpha_half;
    lp = v + z[3];
    z[3] = lp + v;

    return lp;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter::SetCutoff(float cutoff) {
    cutoff_ = cutoff;
    float T = 1.f / fs_;
    float Tdiv2 = T / 2.0f;
    float two_div_T = 2.0f / T;

    float wd = 2.f * M_PI * cutoff_;
    float wa = two_div_T * fasttan(wd * Tdiv2);
    g = wa * Tdiv2;
    gp1 = 1.0f + g;

    // Pre-calculate coefficients
    updateCoefficients();
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter::updateCoefficients() {
    // Pre-calculate halfg to avoid repeated multiplication
    halfg = 0.5f * g;

    // Calculate G coefficients (these only depend on g and gp1)
    G[3] = halfg / gp1;
    G[2] = halfg / (gp1 - halfg * G[3]);
    G[1] = halfg / (gp1 - halfg * G[2]);
    G[0] = g / (gp1 - g * G[1]);
    GAMMA = G[3] * G[2] * G[1] * G[0];

    // Calculate SG coefficients
    SG[0] = G[3] * G[2] * G[1];
    SG[1] = G[3] * G[2];
    SG[2] = G[3];
    SG[3] = 1.0f;

    // Calculate alpha
    alpha = g / gp1;
    alpha_half = alpha * 0.5f;  // Pre-calculate for stages 2-4

    // Calculate beta coefficients
    beta[0] = 1.0f / (gp1 - g * G[1]);
    beta[1] = 1.0f / (gp1 - halfg * G[2]);
    beta[2] = 1.0f / (gp1 - halfg * G[3]);
    beta[3] = 1.0f / gp1;

    // Calculate gamma coefficients
    gamma[0] = 1.0f + G[0] * G[1];
    gamma[1] = 1.0f + G[1] * G[2];
    gamma[2] = 1.0f + G[2] * G[3];

    // Calculate delta coefficients
    delta[0] = g;
    delta[1] = delta[2] = halfg;

    // Calculate epsilon coefficients
    epsilon[0] = G[1];
    epsilon[1] = G[2];
    epsilon[2] = G[3];

    // Pre-calculate kval_ * GAMMA and reciprocal for division elimination
    kval_GAMMA = kval_ * GAMMA;
    reciprocal_kval_GAMMA = 1.0f / (1.0f + kval_GAMMA);
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter::SetResonance(float resonance) {
    kval_ = resonance * 16.998f;
    kval_GAMMA = kval_ * GAMMA;
    reciprocal_kval_GAMMA = 1.0f / (1.0f + kval_GAMMA);
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter::SetSampleRate(float fs) {
    fs_ = fs;
}
