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

#include "ctagSoundProcessorMIDifu.hpp"
#include <iostream>
#include "helpers/ctagFastMath.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void ctagSoundProcessorMIDifu::Init(std::size_t const &blockSize, void *const blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    fx_buffer = (float *) heap_caps_malloc(8192 * sizeof(float),
                                           MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (fx_buffer == NULL) {
        ESP_LOGE("MIDifu", "Could not allocate shared buffer!");
    }
    fx.Init(fx_buffer);
    fx.Clear();
}

void ctagSoundProcessorMIDifu::Process(const ProcessData &data) {
    float fTime = time / 4095.f;
    float fAmount = amount / 4095.f;

    if (cv_time != -1) {
        fTime = HELPERS::fastfabs(data.cv[cv_time]);
    }
    if (cv_amount != -1) {
        fAmount = HELPERS::fastfabs(data.cv[cv_amount]);
    }

    float io[bufSz];
    for (int i = 0; i < bufSz; i++) {
        io[i] = data.buf[i * 2 + processCh];
    }
    fx.Process(fAmount, fTime, io, bufSz);
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] = io[i];
    }
}

ctagSoundProcessorMIDifu::~ctagSoundProcessorMIDifu() {
    heap_caps_free(fx_buffer);
}

void ctagSoundProcessorMIDifu::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("amount", [&](const int val) { amount = val; });
    pMapCv.emplace("amount", [&](const int val) { cv_amount = val; });
    pMapPar.emplace("time", [&](const int val) { time = val; });
    pMapCv.emplace("time", [&](const int val) { cv_time = val; });
    isStereo = false;
    id = "MIDifu";
    // sectionCpp0
}
