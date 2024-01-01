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


#include "ctagSoundProcessorPNoise.hpp"
#include <iostream>
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void ctagSoundProcessorPNoise::Init(std::size_t const &blockSize, void *const blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
}

void ctagSoundProcessorPNoise::Process(const ProcessData &data) {
    fFactor = factor / (float) 1023.f;
    if (cv_factor != -1) {
        fFactor = data.cv[cv_factor];
        fFactor *= fFactor;
    }
    bool ena = enable;
    if(trig_enable != -1) ena = data.trig[trig_enable];
    if(ena){
        for (uint32_t i = 0; i < bufSz; i++) {
            data.buf[i * 2 + processCh] = noiseGen.Process() * fFactor * fFactor;
        }
    }
}

void ctagSoundProcessorPNoise::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("enable", [&](const int val) { enable = val; });
    pMapTrig.emplace("enable", [&](const int val) { trig_enable = val; });
    pMapPar.emplace("factor", [&](const int val) { factor = val; });
    pMapCv.emplace("factor", [&](const int val) { cv_factor = val; });
    isStereo = false;
    id = "PNoise";
    // sectionCpp0
}
