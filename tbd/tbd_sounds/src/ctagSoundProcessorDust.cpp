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


#include <tbd/sounds/ctagSoundProcessorDust.hpp>
#include <iostream>
#include <cmath>
#include "dsps_biquad_gen.h"
#include "dsps_biquad.h"
#include "helpers/ctagFastMath.hpp"


using namespace CTAG::SP;

void ctagSoundProcessorDust::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);


    dust.SetParams(0.5f, 1.0f, 0.0f, 44100.f);

    for (uint32_t i = 0; i < 6; i++) {
        coeffs[i] = 1.f;
    }
    CTAG::SP::HELPERS::dsps_biquad_gen_bpf0db_f32(coeffs, 0.002, 0.4f);
    w1[0] = 0.f;
    w1[1] = 0.f;
    w2[0] = 0.f;
    w2[1] = 0.f;
}

void ctagSoundProcessorDust::Process(const ProcessData &data) {
    float fRate = (float) rate / 100000.f;
    if (cv_rate != -1) {
        fRate = data.cv[cv_rate];
    }
    float tmp[bufSz];
    fRate *= fRate;
    fRate *= 44100.f;
    dust.SetRate(fRate);
    dust.SetBipolar(bipolar);
    float fSmooth = (float) smooth / 4095.f;
    if (cv_smooth != -1) {
        fSmooth = fabs(data.cv[cv_smooth]);
    }
    dust.SetSmooth(fSmooth);
    float fWidth = (float) width / 4095.f * 44100.f * 0.002f;
    if (cv_width != -1) {
        fWidth = fabs(data.cv[cv_width] * 44100.f * 0.002f);
    }
    dust.SetWidth((uint32_t) fWidth);
    float fBPCut = (float) bp_fcut / 44100.f;
    if (cv_bp_fcut != -1) {
        fBPCut = data.cv[cv_bp_fcut];
        fBPCut *= fBPCut;
        fBPCut *= 10000.f / 44100.f;
    }
    float fBPQ = (float) bp_q / 4095.f * 100.f;
    if (cv_bp_q != -1) {
        fBPQ = data.cv[cv_bp_q];
        fBPQ *= fBPQ;
        fBPQ *= 100.f;
    }
    CTAG::SP::HELPERS::dsps_biquad_gen_bpf0db_f32(coeffs, fBPCut, fBPQ);

    for (uint32_t i = 0; i < bufSz; i++) {
        tmp[i] = dust.Process();
    }
    if (bp_enable) {
        if (trig_bp_enable != -1) {
            if (data.trig[trig_bp_enable] == 1)
                dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        } else {
            dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        }
    } else {
        if (trig_bp_enable != -1) {
            if (data.trig[trig_bp_enable] == 0)
                dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        }
    }

    // cascade
    //dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w2);
    float fLevel = (float) level / 4095.f;
    if (cv_level != -1) {
        fLevel = data.cv[cv_level];
    }
    fLevel *= fLevel;
    for (uint32_t i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] = fLevel * tmp[i];
    }
}

void ctagSoundProcessorDust::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("bipolar", [&](const int val) { bipolar = val; });
    pMapTrig.emplace("bipolar", [&](const int val) { trig_bipolar = val; });
    pMapPar.emplace("rate", [&](const int val) { rate = val; });
    pMapCv.emplace("rate", [&](const int val) { cv_rate = val; });
    pMapPar.emplace("level", [&](const int val) { level = val; });
    pMapCv.emplace("level", [&](const int val) { cv_level = val; });
    pMapPar.emplace("width", [&](const int val) { width = val; });
    pMapCv.emplace("width", [&](const int val) { cv_width = val; });
    pMapPar.emplace("smooth", [&](const int val) { smooth = val; });
    pMapCv.emplace("smooth", [&](const int val) { cv_smooth = val; });
    pMapPar.emplace("bp_enable", [&](const int val) { bp_enable = val; });
    pMapTrig.emplace("bp_enable", [&](const int val) { trig_bp_enable = val; });
    pMapPar.emplace("bp_fcut", [&](const int val) { bp_fcut = val; });
    pMapCv.emplace("bp_fcut", [&](const int val) { cv_bp_fcut = val; });
    pMapPar.emplace("bp_q", [&](const int val) { bp_q = val; });
    pMapCv.emplace("bp_q", [&](const int val) { cv_bp_q = val; });
    isStereo = false;
    id = "Dust";
    // sectionCpp0
}
