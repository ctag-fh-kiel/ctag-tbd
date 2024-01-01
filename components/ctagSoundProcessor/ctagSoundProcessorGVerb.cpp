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
// Created by Robert Manzke on 10.02.20.
//

#include "ctagSoundProcessorGVerb.hpp"
#include <iostream>
#include <cmath>
#include "esp_log.h"

using namespace CTAG::SP;

void ctagSoundProcessorGVerb::Init(std::size_t const &blockSize, void *const blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    maxRoomSize = 500.f;
    gverb = (ty_gverb *) malloc(sizeof(ty_gverb));
    if (gverb == nullptr) {
        ESP_LOGE("GVERB", "Fatal error, couldn't allocate memory!");
    }
    // roomSz, time, damp, spread, ibw, early, tail
    gverb_new(gverb, 44100, maxRoomSize, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
}

void ctagSoundProcessorGVerb::Process(const ProcessData &data) {
    float revout_l, revout_r;
    fRoomSz = (float) roomsize / 4095.f * maxRoomSize;
    fRevTime = (float) revtime / 4095.f * 30.f;
    fDamping = (float) damping / 4095.f - 0.01f;
    fInputBW = (float) inputbw / 4095.f;
    fEarlyLvl = (float) earlylvl / 4095.f;
    fTailLvl = (float) taillvl / 4095.f;
    fDryWet = (float) drywet / 4095.f;
//    if(cvRoomSz != -1) fRoomSz = data.cv[cvRoomSz] * data.cv[cvRoomSz] * maxRoomSize;
    gverb_set_roomsize(gverb, fRoomSz);
    if (cv_revtime != -1) fRevTime = fabsf(data.cv[cv_revtime]) * 30.f;
    gverb_set_revtime(gverb, fRevTime);
    if (cv_damping != -1) fDamping = (data.cv[cv_damping] * data.cv[cv_damping]) - 0.01f;
    gverb_set_damping(gverb, fDamping);
    if (cv_inputbw != -1) fInputBW = data.cv[cv_inputbw] * data.cv[cv_inputbw];
    gverb_set_inputbandwidth(gverb, fInputBW);
    if (cv_earlylvl != -1) fEarlyLvl = data.cv[cv_earlylvl] * data.cv[cv_earlylvl];
    gverb_set_earlylevel(gverb, fEarlyLvl);
    if (cv_taillvl != -1) fTailLvl = data.cv[cv_taillvl] * data.cv[cv_taillvl];
    gverb_set_taillevel(gverb, fTailLvl);
    if (cv_drywet != -1) fDryWet = data.cv[cv_drywet] * data.cv[cv_drywet];
    for (int i = 0; i < bufSz; i++) {
        if (mono) {
            gverb_do(gverb, data.buf[i * 2], &revout_l, &revout_r);
        } else {
            gverb_do(gverb, (data.buf[i * 2] + data.buf[i * 2 + 1]) * 0.5f, &revout_l, &revout_r);
        }
        data.buf[i * 2] = fDryWet * revout_l + (1.f - fDryWet) * data.buf[i * 2];
        if (mono) {
            data.buf[i * 2 + 1] = fDryWet * revout_r + (1.f - fDryWet) * data.buf[i * 2];
        } else {
            data.buf[i * 2 + 1] = fDryWet * revout_r + (1.f - fDryWet) * data.buf[i * 2 + 1];
        }
    }
}

ctagSoundProcessorGVerb::~ctagSoundProcessorGVerb() {
    // careful, this also frees itself
    gverb_free(gverb);
}

void ctagSoundProcessorGVerb::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("roomsize", [&](const int val) { roomsize = val; });
    pMapCv.emplace("roomsize", [&](const int val) { cv_roomsize = val; });
    pMapPar.emplace("revtime", [&](const int val) { revtime = val; });
    pMapCv.emplace("revtime", [&](const int val) { cv_revtime = val; });
    pMapPar.emplace("damping", [&](const int val) { damping = val; });
    pMapCv.emplace("damping", [&](const int val) { cv_damping = val; });
    pMapPar.emplace("inputbw", [&](const int val) { inputbw = val; });
    pMapCv.emplace("inputbw", [&](const int val) { cv_inputbw = val; });
    pMapPar.emplace("earlylvl", [&](const int val) { earlylvl = val; });
    pMapCv.emplace("earlylvl", [&](const int val) { cv_earlylvl = val; });
    pMapPar.emplace("taillvl", [&](const int val) { taillvl = val; });
    pMapCv.emplace("taillvl", [&](const int val) { cv_taillvl = val; });
    pMapPar.emplace("drywet", [&](const int val) { drywet = val; });
    pMapCv.emplace("drywet", [&](const int val) { cv_drywet = val; });
    pMapPar.emplace("mono", [&](const int val) { mono = val; });
    pMapTrig.emplace("mono", [&](const int val) { trig_mono = val; });
    isStereo = true;
    id = "GVerb";
    // sectionCpp0
}
