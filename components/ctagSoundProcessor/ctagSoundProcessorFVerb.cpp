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

#include "ctagSoundProcessorFVerb.hpp"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

void ctagSoundProcessorFVerb::Init() {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
}

void ctagSoundProcessorFVerb::Process(const ProcessData &data) {
    float val = (float) roomsize / 4095.f;
    if (cv_roomsize != -1) val = data.cv[cv_roomsize] * data.cv[cv_roomsize];
    freeverb.setroomsize(val);
    val = (float) damp / 4095.f;
    if (cv_damp != -1) val = data.cv[cv_damp] * data.cv[cv_damp];
    freeverb.setdamp(val);
    val = (float) dry / 4095.f;
    if (cv_dry != -1) val = data.cv[cv_dry] * data.cv[cv_dry];
    freeverb.setdry(val);
    val = (float) width / 4095.f;
    if (cv_width != -1) val = data.cv[cv_width] * data.cv[cv_width];
    freeverb.setwidth(val);
    val = (float) mode;
    if (trig_mode != -1) {
        val = 1.f - data.trig[trig_mode];
    }
    freeverb.setmode(val);
    freeverb.setmono(mono);
    val = (float) wet / 4095.f;
    if (cv_wet != -1) val = data.cv[cv_wet] * data.cv[cv_wet];
    freeverb.setwet(val);
    freeverb.process(data.buf, bufSz);
}

void ctagSoundProcessorFVerb::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("roomsize", [&](const int val) { roomsize = val; });
    pMapCv.emplace("roomsize", [&](const int val) { cv_roomsize = val; });
    pMapPar.emplace("damp", [&](const int val) { damp = val; });
    pMapCv.emplace("damp", [&](const int val) { cv_damp = val; });
    pMapPar.emplace("dry", [&](const int val) { dry = val; });
    pMapCv.emplace("dry", [&](const int val) { cv_dry = val; });
    pMapPar.emplace("wet", [&](const int val) { wet = val; });
    pMapCv.emplace("wet", [&](const int val) { cv_wet = val; });
    pMapPar.emplace("width", [&](const int val) { width = val; });
    pMapCv.emplace("width", [&](const int val) { cv_width = val; });
    pMapPar.emplace("mode", [&](const int val) { mode = val; });
    pMapTrig.emplace("mode", [&](const int val) { trig_mode = val; });
    pMapPar.emplace("mono", [&](const int val) { mono = val; });
    pMapTrig.emplace("mono", [&](const int val) { trig_mono = val; });
    isStereo = true;
    id = "FVerb";
    // sectionCpp0
}

