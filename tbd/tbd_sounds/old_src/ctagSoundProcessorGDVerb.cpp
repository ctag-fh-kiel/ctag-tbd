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
// Created by Robert Manzke on 13.02.20.
//

#include "ctagSoundProcessorGDVerb.hpp"
#include "esp_heap_caps.h"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

void ctagSoundProcessorGDVerb::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    strev.init(blockSize, blockPtr);
    strev.setSampleRate(44100.f);
    strev.mute();
}

void ctagSoundProcessorGDVerb::Process(const ProcessData &data) {
    float fRevTime = (float) revtime / 4095.f * 20.f;
    if(cv_revtime != -1){
        fRevTime = 0.9f * prefRevTime + 0.1f * (0.2f + fabsf(data.cv[cv_revtime]) * 20.f);
        prefRevTime = fRevTime;
    }
    strev.setrt60(fRevTime);
    strev.setdccutfreq((float) dccut);
    strev.setidiffusion1((float) idiffusion1 / 4095.f);
    strev.setidiffusion2((float) idiffusion2 / 4095.f);
    strev.setdiffusion1((float) diffusion1 / 4095.f);
    strev.setdiffusion2((float) diffusion2 / 4095.f);
    strev.setinputdamp((float) inputdamp);
    strev.setdamp((float) damp);
    strev.setoutputdamp((float) outputdamp);
    strev.setspin((float) spin / 4095.f * 10.f);
    strev.setspindiff((float) spindiff / 4095.f);
    strev.setspinlimit((float) spinlimit / 4095.f * 50.f);
    strev.setwander((float) wander / 4095.f);
    strev.setmodulationnoise1((float) modnoise1 / 4095.f * 0.1f);
    strev.setmodulationnoise2((float) modnoise2 / 4095.f * 0.1f);
    strev.setAutoDiff(autodiff);
    strev.setwetr((float) wet / 4095.f);
    strev.setdryr((float) dry / 4095.f);
    strev.setwidth((float) width / 4095.f);
    strev.setmono((bool) mono);

    if (cv_width != -1) {
        strev.setwidth(fabsf(data.cv[cv_width]));
    }
    if (cv_wet != -1) {
        strev.setwetr(fabs(data.cv[cv_wet]));
    }
    if (cv_dry != -1) {
        strev.setdryr(fabs(data.cv[cv_dry]));
    }
    if (cv_dccut != -1) {
        strev.setdccutfreq(data.cv[cv_dccut] * data.cv[cv_dccut] * 10000.f);
    }
    if (cv_inputdamp != -1) {
        strev.setinputdamp(data.cv[cv_inputdamp] * data.cv[cv_inputdamp] * 20000.f);
    }
    if (cv_damp != -1) {
        strev.setdamp(data.cv[cv_damp] * data.cv[cv_damp] * 20000.f);
    }
    if (cv_outputdamp != -1) {
        strev.setoutputdamp(data.cv[cv_outputdamp] * data.cv[cv_outputdamp] * 20000.f);
    }
    if (cv_wander != -1) {
        strev.setwander(data.cv[cv_wander] * data.cv[cv_wander]);
    }

    // process the data
    strev.processreplace(data.buf, bufSz);
}

void ctagSoundProcessorGDVerb::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("revtime", [&](const int val) { revtime = val; });
    pMapCv.emplace("revtime", [&](const int val) { cv_revtime = val; });
    pMapPar.emplace("dccut", [&](const int val) { dccut = val; });
    pMapCv.emplace("dccut", [&](const int val) { cv_dccut = val; });
    pMapPar.emplace("idiffusion1", [&](const int val) { idiffusion1 = val; });
    pMapCv.emplace("idiffusion1", [&](const int val) { cv_idiffusion1 = val; });
    pMapPar.emplace("idiffusion2", [&](const int val) { idiffusion2 = val; });
    pMapCv.emplace("idiffusion2", [&](const int val) { cv_idiffusion2 = val; });
    pMapPar.emplace("diffusion1", [&](const int val) { diffusion1 = val; });
    pMapCv.emplace("diffusion1", [&](const int val) { cv_diffusion1 = val; });
    pMapPar.emplace("diffusion2", [&](const int val) { diffusion2 = val; });
    pMapCv.emplace("diffusion2", [&](const int val) { cv_diffusion2 = val; });
    pMapPar.emplace("inputdamp", [&](const int val) { inputdamp = val; });
    pMapCv.emplace("inputdamp", [&](const int val) { cv_inputdamp = val; });
    pMapPar.emplace("damp", [&](const int val) { damp = val; });
    pMapCv.emplace("damp", [&](const int val) { cv_damp = val; });
    pMapPar.emplace("outputdamp", [&](const int val) { outputdamp = val; });
    pMapCv.emplace("outputdamp", [&](const int val) { cv_outputdamp = val; });
    pMapPar.emplace("spin", [&](const int val) { spin = val; });
    pMapCv.emplace("spin", [&](const int val) { cv_spin = val; });
    pMapPar.emplace("spindiff", [&](const int val) { spindiff = val; });
    pMapCv.emplace("spindiff", [&](const int val) { cv_spindiff = val; });
    pMapPar.emplace("spinlimit", [&](const int val) { spinlimit = val; });
    pMapCv.emplace("spinlimit", [&](const int val) { cv_spinlimit = val; });
    pMapPar.emplace("wander", [&](const int val) { wander = val; });
    pMapCv.emplace("wander", [&](const int val) { cv_wander = val; });
    pMapPar.emplace("modnoise1", [&](const int val) { modnoise1 = val; });
    pMapCv.emplace("modnoise1", [&](const int val) { cv_modnoise1 = val; });
    pMapPar.emplace("modnoise2", [&](const int val) { modnoise2 = val; });
    pMapCv.emplace("modnoise2", [&](const int val) { cv_modnoise2 = val; });
    pMapPar.emplace("autodiff", [&](const int val) { autodiff = val; });
    pMapTrig.emplace("autodiff", [&](const int val) { trig_autodiff = val; });
    pMapPar.emplace("dry", [&](const int val) { dry = val; });
    pMapCv.emplace("dry", [&](const int val) { cv_dry = val; });
    pMapPar.emplace("wet", [&](const int val) { wet = val; });
    pMapCv.emplace("wet", [&](const int val) { cv_wet = val; });
    pMapPar.emplace("width", [&](const int val) { width = val; });
    pMapCv.emplace("width", [&](const int val) { cv_width = val; });
    pMapPar.emplace("mono", [&](const int val) { mono = val; });
    pMapTrig.emplace("mono", [&](const int val) { trig_mono = val; });
    isStereo = true;
    id = "GDVerb";
    // sectionCpp0
}
