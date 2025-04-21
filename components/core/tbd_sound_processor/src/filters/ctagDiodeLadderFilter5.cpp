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
#include <tbd/sound_utils/filters/ctagDiodeLadderFilter5.hpp>
#include <tbd/helpers/ctagFastMath.hpp>

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::Init() {
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

    xh = 0.f;
    H0 = 1.f;

    gain_ = 1.5f;

    SetResonance(1.f);
    SetCutoff(1000.f);
}

float CTAG::SP::HELPERS::ctagDiodeLadderFilter5::Process(float in) {
    // boost low frequencies 1st oder low shelving according to Zoelzer
    // offsets signal loss at higher resonances
    float xh_new = in + cb * xh;
    float ap_y = cb * xh_new + xh;
    xh = xh_new;
    in = 0.5f * H0 * (in + ap_y) + in;

    // diode ladder
    G[3] = 0.5f * g / gp1;
    G[2] = 0.5f * g / (gp1 - 0.5f * g * G[3]);
    G[1] = 0.5f * g / (gp1 - 0.5f * g * G[2]);
    G[0] = g / (gp1 - g * G[1]);
    GAMMA = G[3] * G[2] * G[1] * G[0];

    SG[0] = G[3] * G[2] * G[1];
    SG[1] = G[3] * G[2];
    SG[2] = G[3];
    SG[3] = 1.0f;

    alpha = g / gp1;

    beta[0] = 1.0f / (gp1 - g * G[1]);
    beta[1] = 1.0f / (gp1 - 0.5 * g * G[2]);
    beta[2] = 1.0f / (gp1 - 0.5 * g * G[3]);
    beta[3] = 1.0f / gp1;

    gamma[0] = 1.0f + G[0] * G[1];
    gamma[1] = 1.0f + G[1] * G[2];
    gamma[2] = 1.0f + G[2] * G[3];

    delta[0] = g;
    delta[1] = delta[2] = 0.5f * g;

    epsilon[0] = G[1];
    epsilon[1] = G[2];
    epsilon[2] = G[3];

    //feedback inputs
    float fb4 = beta[3] * z[3];
    float fb3 = beta[2] * (z[2] + fb4 * delta[2]);
    float fb2 = beta[1] * (z[1] + fb3 * delta[1]);

    //feedback process
    float fbo1 = (beta[0] * (z[0] + fb2 * delta[0]));
    float fbo2 = (beta[1] * (z[1] + fb3 * delta[1]));
    float fbo3 = (beta[2] * (z[2] + fb4 * delta[2]));

    SIGMA = (SG[0] * fbo1) + (SG[1] * fbo2) + (SG[2] * fbo3) + (SG[3] * fb4);
    // non linearity goes to one call up
    //in = oneovtanh * fasttanh(saturation_ * in);
    //in = stmlib::SoftLimit(saturation_ * in);
    //in = fasttanh(saturation * in);

    // form input to loop
    float un = (in - kval_ * SIGMA) / (1.0f + kval_ * GAMMA);

    // 1st stage
    float xin = un * gamma[0] + fb2 + epsilon[0] * fbo1;
    float v = (a[0] * xin - z[0]) * alpha;
    float lp = v + z[0];
    z[0] = lp + v;

    // 2nd stage
    xin = lp * gamma[1] + fb3 + epsilon[1] * fbo2;
    v = (a[1] * xin - z[1]) * alpha;
    lp = v + z[1];
    z[1] = lp + v;

    // 3rd stage
    xin = lp * gamma[2] + fb4 + epsilon[2] * fbo3;
    v = (a[2] * xin - z[2]) * alpha;
    lp = v + z[2];
    z[2] = lp + v;

    // 4th stage
    v = (a[3] * lp - z[3]) * alpha;
    lp = v + z[3];
    z[3] = lp + v;

    return lp;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::SetCutoff(float cutoff) {
    cutoff_ = cutoff;
    float T = 1.f / fs_;
    float Tdiv2 = T / 2.0f;

    float tn = fasttan(M_PI * cutoff / fs_);
    float wa = 2.f * fs_ * tn;
    g = wa * Tdiv2;
    gp1 = 1.0f + g;

    // low shelf boost
    float tnls = tn;
    if (cutoff > 10000.f) { // limit low shelf frequency, this is fixed for 44100kHz to avoid extra tan calculation
        tnls = 0.012434004797755f;
    }
    cb = (tnls - 1.f) / (tnls + 1.f);

}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::SetResonance(float resonance) {
    resonance_ = resonance;
    kval_ = resonance * 16.5f;
    calcLowShelveBoost();
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::SetSampleRate(float fs) {
    fs_ = fs;
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::SetGain(float gain) {
    ctagFilterBase::SetGain(gain);
    calcLowShelveBoost();
}

void CTAG::SP::HELPERS::ctagDiodeLadderFilter5::calcLowShelveBoost() {
    H0 = ((kval_ < 1.f ? 1.f : kval_) * gain_) - 1.f; // this is approximate by hearing
    if (H0 < 0.f) H0 = 0.f;
}
