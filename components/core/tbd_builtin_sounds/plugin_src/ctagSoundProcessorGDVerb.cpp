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

#include <tbd/sounds/SoundProcessorGDVerb.hpp>
#include <iostream>
#include <cmath>

using namespace tbd::sounds;

void SoundProcessorGDVerb::Init(std::size_t blockSize, void *blockPtr) {
    strev.init(blockSize, blockPtr);
    strev.setSampleRate(44100.f);
    strev.mute();
}

void SoundProcessorGDVerb::Process(const sound_processor::ProcessData&data) {
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
