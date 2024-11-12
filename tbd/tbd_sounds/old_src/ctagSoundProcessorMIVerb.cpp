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

#include "ctagSoundProcessorMIVerb.hpp"
#include <iostream>
#include "esp_heap_caps.h"
#include "helpers/ctagFastMath.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void ctagSoundProcessorMIVerb::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    reverb_buffer = (float *) heap_caps_malloc(32768 * sizeof(float),
                                               MALLOC_CAP_SPIRAM);//MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (reverb_buffer == NULL) {
        ESP_LOGE("MIVerb", "Could not allocate shared buffer!");
    }

    reverb.Init(reverb_buffer);
    reverb.set_amount(0.5f);
    reverb.set_lp(0.5f);
    reverb.set_lfo2_freq(0.1f);
    reverb.set_lfo1_freq(0.2f);
    reverb.set_time(0.2f);
    reverb.set_diffusion(0.5f);
}

void ctagSoundProcessorMIVerb::Process(const ProcessData &data) {

    float fTime = time / 4095.f;
    float fDiffusion = diffusion / 4095.f;
    float fLp = lp / 4095.f;
    float fGain = in_gain / 4095.f;
    float fAmount = amount / 4095.f;
    float lfo1 = lfo1_f / 4095.f;
    float lfo2 = lfo2_f / 4095.f;

    if (cv_time != -1) {
        fTime = HELPERS::fastfabs(data.cv[cv_time]);
    }
    if (cv_diffusion != -1) {
        fDiffusion = HELPERS::fastfabs(data.cv[cv_diffusion]);
    }
    if (cv_lp != -1) {
        fLp = HELPERS::fastfabs(data.cv[cv_lp]);
    }
    if (cv_in_gain != -1) {
        fGain = HELPERS::fastfabs(data.cv[cv_in_gain]);
    }
    if (cv_amount != -1) {
        fAmount = HELPERS::fastfabs(data.cv[cv_amount]);
    }
    if (cv_lfo1_f != -1) {
        lfo1 = HELPERS::fastfabs(data.cv[cv_lfo1_f]);
    }
    if (cv_lfo2_f != -1) {
        lfo2 = HELPERS::fastfabs(data.cv[cv_lfo2_f]);
    }

    reverb.set_diffusion(fDiffusion);
    reverb.set_input_gain(fGain);
    reverb.set_amount(fAmount);
    reverb.set_lp(fLp);
    reverb.set_time(fTime);
    reverb.set_lfo1_freq(lfo1);
    reverb.set_lfo2_freq(lfo2);

    float left[bufSz], right[bufSz];
    for (int i = 0; i < bufSz; i++) {
        left[i] = data.buf[i * 2];
        right[i] = data.buf[i * 2 + 1];
    }

    reverb.Process(left, right, bufSz);

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = left[i];
        data.buf[i * 2 + 1] = right[i];
    }
}

ctagSoundProcessorMIVerb::~ctagSoundProcessorMIVerb() {
    heap_caps_free(reverb_buffer);
}

void ctagSoundProcessorMIVerb::knowYourself() {
// sectionCpp0
    pMapPar.emplace("time", [&](const int val) { time = val; });
    pMapCv.emplace("time", [&](const int val) { cv_time = val; });
    pMapPar.emplace("amount", [&](const int val) { amount = val; });
    pMapCv.emplace("amount", [&](const int val) { cv_amount = val; });
    pMapPar.emplace("in_gain", [&](const int val) { in_gain = val; });
    pMapCv.emplace("in_gain", [&](const int val) { cv_in_gain = val; });
    pMapPar.emplace("diffusion", [&](const int val) { diffusion = val; });
    pMapCv.emplace("diffusion", [&](const int val) { cv_diffusion = val; });
    pMapPar.emplace("lp", [&](const int val) { lp = val; });
    pMapCv.emplace("lp", [&](const int val) { cv_lp = val; });
    pMapPar.emplace("lfo1_f", [&](const int val) { lfo1_f = val; });
    pMapCv.emplace("lfo1_f", [&](const int val) { cv_lfo1_f = val; });
    pMapPar.emplace("lfo2_f", [&](const int val) { lfo2_f = val; });
    pMapCv.emplace("lfo2_f", [&](const int val) { cv_lfo2_f = val; });
    isStereo = true;
    id = "MIVerb";
    // sectionCpp0
}
