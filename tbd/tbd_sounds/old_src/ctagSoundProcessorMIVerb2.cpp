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

#include "ctagSoundProcessorMIVerb2.hpp"
#include <iostream>
#include "clouds/dsp/frame.h"
#include "helpers/ctagFastMath.hpp"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void ctagSoundProcessorMIVerb2::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    reverb_buffer = (float *) heaps::malloc(32768 * sizeof(float),
                                               MALLOC_CAP_SPIRAM);//MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (reverb_buffer == NULL) {
        TBD_LOGE("MIVerb", "Could not allocate shared buffer!");
    }

    reverb.Init(reverb_buffer);
}

void ctagSoundProcessorMIVerb2::Process(const ProcessData &data) {
    float fDecay = decay / 4095.f;
    if (cv_decay != -1) {
        fDecay = HELPERS::fastfabs(data.cv[cv_decay]);
    }
    float fSize = size / 4095.f;
    if (cv_size != -1) {
        fSize = HELPERS::fastfabs(data.cv[cv_size]);
    }
    float fGain = in_gain / 4095.f;
    if (cv_in_gain != -1) {
        fGain = HELPERS::fastfabs(data.cv[cv_in_gain]);
    }
    float fDiffusion = diffusion / 4095.f;
    if (cv_diffusion != -1) {
        fDiffusion = HELPERS::fastfabs(data.cv[cv_diffusion]);
    }
    float fLp = lp / 4095.f;
    if (cv_lp != -1) {
        fLp = HELPERS::fastfabs(data.cv[cv_lp]);
    }
    float fHp = hp / 4095.f;
    if (cv_hp != -1) {
        fHp = HELPERS::fastfabs(data.cv[cv_hp]);
    }
    float fModAmt = mod_amount / 4095.f;
    if (cv_mod_amount != -1) {
        fModAmt = HELPERS::fastfabs(data.cv[cv_mod_amount]);
    }
    float fModRate = mod_rate / 4095.f;
    if (cv_mod_rate != -1) {
        fModRate = HELPERS::fastfabs(data.cv[cv_mod_rate]);
    }
    float fRatio = ratio / 4095.f;
    if (cv_ratio != -1) {
        fRatio = HELPERS::fastfabs(data.cv[cv_ratio]);
    }
    float fPShftAmt = p_shift_amt / 4095.f;
    if (cv_p_shift_amt != -1) {
        fPShftAmt = HELPERS::fastfabs(data.cv[cv_p_shift_amt]);
    }
    float fAmount = amount / 4095.f;
    if (cv_amount != -1) {
        fAmount = HELPERS::fastfabs(data.cv[cv_amount]);
    }

    reverb.set_decay(fDecay);
    reverb.set_ratio(fRatio);
    reverb.set_size(fSize);
    reverb.set_diffusion(fDiffusion);
    reverb.set_lp(fLp);
    reverb.set_input_gain(fGain);
    reverb.set_hp(fHp);
    reverb.set_mod_amount(fModAmt);
    reverb.set_mod_rate(fModRate);
    reverb.set_pitch_shift_amount(fPShftAmt);
    reverb.set_amount(fAmount);

    clouds::FloatFrame frames[bufSz];
    for (int i = 0; i < bufSz; i++) {
        frames[i].l = data.buf[i * 2];
        frames[i].r = data.buf[i * 2 + 1];
    }

    reverb.Process(frames, bufSz);

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = frames[i].l;
        data.buf[i * 2 + 1] = frames[i].r;
    }

}

ctagSoundProcessorMIVerb2::~ctagSoundProcessorMIVerb2() {
    heaps::free(reverb_buffer);
}

void ctagSoundProcessorMIVerb2::knowYourself() {
    // autogenerated code here
// sectionCpp0
    pMapPar.emplace("decay", [&](const int val) { decay = val; });
    pMapCv.emplace("decay", [&](const int val) { cv_decay = val; });
    pMapPar.emplace("amount", [&](const int val) { amount = val; });
    pMapCv.emplace("amount", [&](const int val) { cv_amount = val; });
    pMapPar.emplace("size", [&](const int val) { size = val; });
    pMapCv.emplace("size", [&](const int val) { cv_size = val; });
    pMapPar.emplace("in_gain", [&](const int val) { in_gain = val; });
    pMapCv.emplace("in_gain", [&](const int val) { cv_in_gain = val; });
    pMapPar.emplace("diffusion", [&](const int val) { diffusion = val; });
    pMapCv.emplace("diffusion", [&](const int val) { cv_diffusion = val; });
    pMapPar.emplace("lp", [&](const int val) { lp = val; });
    pMapCv.emplace("lp", [&](const int val) { cv_lp = val; });
    pMapPar.emplace("hp", [&](const int val) { hp = val; });
    pMapCv.emplace("hp", [&](const int val) { cv_hp = val; });
    pMapPar.emplace("mod_amount", [&](const int val) { mod_amount = val; });
    pMapCv.emplace("mod_amount", [&](const int val) { cv_mod_amount = val; });
    pMapPar.emplace("mod_rate", [&](const int val) { mod_rate = val; });
    pMapCv.emplace("mod_rate", [&](const int val) { cv_mod_rate = val; });
    pMapPar.emplace("ratio", [&](const int val) { ratio = val; });
    pMapCv.emplace("ratio", [&](const int val) { cv_ratio = val; });
    pMapPar.emplace("p_shift_amt", [&](const int val) { p_shift_amt = val; });
    pMapCv.emplace("p_shift_amt", [&](const int val) { cv_p_shift_amt = val; });
    isStereo = true;
    id = "MIVerb2";
    // sectionCpp0
}
