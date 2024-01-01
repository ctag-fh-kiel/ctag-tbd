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

#include "ctagSoundProcessorGDVerb2.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <iostream>
#include <cmath>
#include "stmlib/stmlib.h"

using namespace CTAG::SP;

void ctagSoundProcessorGDVerb2::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    prog_rev.setSampleRate(44100.f);
    prog_rev.mute();
    ESP_LOGE("GDVERB2", "os fac %li, total fs %f, fs %f", prog_rev.getOSFactor(), prog_rev.getTotalSampleRate(),
             prog_rev.getSampleRate());
}

void ctagSoundProcessorGDVerb2::Process(const ProcessData &data) {
    // decay
    float fRevTime = revtime / 4095.f * 30.f;
    if(cv_revtime != -1){
        fRevTime = 0.9f * prefRevTime + 0.1f * (0.2f + fabsf(data.cv[cv_revtime]) * 30.f); // smooth
        prefRevTime = fRevTime;
    }
    float fDecay0 = decay0 / 4095.f * 2.f;
    float fDecay1 = decay1 / 4095.f;
    float fDecay2 = decay2 / 4095.f;
    float fDecay3 = decay3 / 4095.f;
    float fDecayf = decayf / 4095.f * 4.f;
    if(!exprtdecay){
        CONSTRAIN(fRevTime, 0.1f, 30.f)
        prog_rev.setrt60(fRevTime);
        prog_rev.setdecay0(0.237);
        prog_rev.setdecay1(0.938);
        prog_rev.setdecay2(0.844);
        prog_rev.setdecay3(0.906);
        prog_rev.setdecayf(1.000);
    }else{
        CONSTRAIN(fRevTime, 0.1f, 30.f)
        prog_rev.setrt60(fRevTime);
        prog_rev.setdecay0(fDecay0);
        prog_rev.setdecay1(fDecay1);
        prog_rev.setdecay2(fDecay2);
        prog_rev.setdecay3(fDecay3);
        prog_rev.setdecayf(fDecayf);
    }
    prog_rev.resetdecay();

    // diffusion
    float fDiffusion1 = diffusion1 / 4095.f;
    float fDiffusion2 = diffusion2 / 4095.f;
    float fDiffusion3 = diffusion3 / 4095.f;
    float fDiffusion4 = diffusion4 / 4095.f;
    if(!exprtdiffu){
        prog_rev.setdiffusion1(0.375f);
        prog_rev.setdiffusion2(0.312f);
        prog_rev.setdiffusion3(0.406f);
        prog_rev.setdiffusion4(0.25f);
    }else{
        prog_rev.setdiffusion1(fDiffusion1);
        prog_rev.setdiffusion2(fDiffusion2);
        prog_rev.setdiffusion3(fDiffusion3);
        prog_rev.setdiffusion4(fDiffusion4);
    }


    // dc cut highpass, damping
    float fDcCut = dccut;
    if(cv_dccut != -1){
        fDcCut = 5.f + fabsf(data.cv[cv_dccut]) * 10000.f;
    }
    float fInputDamp = inputdamp;
    if(cv_inputdamp != -1){
        fInputDamp = 10.f + fabsf(data.cv[cv_inputdamp]) * 20000.f;
    }
    float fDamp = damp;
    if(cv_damp != -1){
        fDamp = 10.f + fabsf(data.cv[cv_damp]) * 20000.f;
    }
    float fDamp2 = damp2;
    float fOutDamp = outputdamp;
    if(cv_outputdamp != -1){
        fOutDamp = 10.f + fabsf(data.cv[cv_outputdamp]) * 11000.f;
    }
    float fOutDampBW = 1.f + outputdampbw / 4095.f * 8.f;
    float fBassBW = 1.f + bassbw / 4095.f * 8.f;
    float fBassBoost = bassboost / 4095.f / 5.f;
    if(!exprtdamp){
        prog_rev.setdccutfreq(5.f);
        prog_rev.setinputdamp(20000.f);
        prog_rev.setdamp(9000.f);
        prog_rev.setoutputdamp(10000.f);
        prog_rev.setoutputdampbw(2.f);
        prog_rev.setdamp2(500.f); // bass boost
        prog_rev.setbassbw(2.f);
        prog_rev.setbassboost(0.1);
    }else{
        prog_rev.setdccutfreq(fDcCut);
        prog_rev.setinputdamp(fInputDamp);
        prog_rev.setdamp(fDamp);
        prog_rev.setdamp2(fDamp2);
        prog_rev.setoutputdamp(fOutDamp);
        prog_rev.setoutputdampbw(fOutDampBW);
        prog_rev.setbassbw(fBassBW);
        prog_rev.setbassboost(fBassBoost);
    }


    // modulation
    float fSpin = spin / 4095.f * 5.f;
    float fSpin2 = spin2 / 4095.f * 5.f;
    float fSpinLimit = spinlimit / 4095.f * 20.f;
    float fSpinLimit2 = spinlimit2 / 4095.f * 20.f;
    float fWander = wander / 4095.f * 0.9f;
    float fWander2 = wander2 / 4095.f * 0.9f;
    float fSpinToWander = spin2wander / 4095.f * 100.f;
    if(!exprtmod){
        prog_rev.setspin(0.5f);
        prog_rev.setspinlimit(20.f);
        prog_rev.setwander(0.5f);
        fSpinToWander = 22.f;
        prog_rev.setspin2(2.4f);
        prog_rev.setspinlimit2(12.f);
        prog_rev.setwander2(0.3f);
    }else{
        prog_rev.setspin(fSpin);
        prog_rev.setspin2(fSpin2);
        prog_rev.setspinlimit(fSpinLimit);
        prog_rev.setspinlimit2(fSpinLimit2);
        prog_rev.setwander(fWander);
        prog_rev.setwander2(fWander2);
    }
    // no CV for this parameter as it involves significant computation
    CONSTRAIN(fSpinToWander, 1.f, 100.f)
    if(fSpinToWander != prefSpinToWander){
        prefSpinToWander = fSpinToWander;
        prog_rev.setspin2wander(fSpinToWander);
    }

    // levels, width mono
    float fDry = dry / 4095.f;
    if(cv_dry != -1){
        fDry = fabsf(data.cv[cv_dry]);
    }
    prog_rev.setdryr(fDry);
    float fWet = wet / 4095.f;
    if(cv_wet != -1){
        fWet = fabsf(data.cv[cv_wet]);
    }
    prog_rev.setwetr(fWet);
    float fWidth = width / 4095.f;
    if(cv_width != -1){
        fWidth = fabsf(data.cv[cv_width]);
    }
    prog_rev.setwidth(fWidth);

    prog_rev.setmono(mono / 4095.f);


    // process the data
    prog_rev.processreplace(data.buf, bufSz);
}

void ctagSoundProcessorGDVerb2::knowYourself() {
    // sectionCpp0
	pMapPar.emplace("exprtdecay", [&](const int val){ exprtdecay = val;});
	pMapTrig.emplace("exprtdecay", [&](const int val){ trig_exprtdecay = val;});
	pMapPar.emplace("revtime", [&](const int val){ revtime = val;});
	pMapCv.emplace("revtime", [&](const int val){ cv_revtime = val;});
	pMapPar.emplace("decay0", [&](const int val){ decay0 = val;});
	pMapCv.emplace("decay0", [&](const int val){ cv_decay0 = val;});
	pMapPar.emplace("decay1", [&](const int val){ decay1 = val;});
	pMapCv.emplace("decay1", [&](const int val){ cv_decay1 = val;});
	pMapPar.emplace("decay2", [&](const int val){ decay2 = val;});
	pMapCv.emplace("decay2", [&](const int val){ cv_decay2 = val;});
	pMapPar.emplace("decay3", [&](const int val){ decay3 = val;});
	pMapCv.emplace("decay3", [&](const int val){ cv_decay3 = val;});
	pMapPar.emplace("decayf", [&](const int val){ decayf = val;});
	pMapCv.emplace("decayf", [&](const int val){ cv_decayf = val;});
	pMapPar.emplace("exprtdiffu", [&](const int val){ exprtdiffu = val;});
	pMapTrig.emplace("exprtdiffu", [&](const int val){ trig_exprtdiffu = val;});
	pMapPar.emplace("diffusion1", [&](const int val){ diffusion1 = val;});
	pMapCv.emplace("diffusion1", [&](const int val){ cv_diffusion1 = val;});
	pMapPar.emplace("diffusion2", [&](const int val){ diffusion2 = val;});
	pMapCv.emplace("diffusion2", [&](const int val){ cv_diffusion2 = val;});
	pMapPar.emplace("diffusion3", [&](const int val){ diffusion3 = val;});
	pMapCv.emplace("diffusion3", [&](const int val){ cv_diffusion3 = val;});
	pMapPar.emplace("diffusion4", [&](const int val){ diffusion4 = val;});
	pMapCv.emplace("diffusion4", [&](const int val){ cv_diffusion4 = val;});
	pMapPar.emplace("exprtdamp", [&](const int val){ exprtdamp = val;});
	pMapTrig.emplace("exprtdamp", [&](const int val){ trig_exprtdamp = val;});
	pMapPar.emplace("dccut", [&](const int val){ dccut = val;});
	pMapCv.emplace("dccut", [&](const int val){ cv_dccut = val;});
	pMapPar.emplace("inputdamp", [&](const int val){ inputdamp = val;});
	pMapCv.emplace("inputdamp", [&](const int val){ cv_inputdamp = val;});
	pMapPar.emplace("damp", [&](const int val){ damp = val;});
	pMapCv.emplace("damp", [&](const int val){ cv_damp = val;});
	pMapPar.emplace("outputdamp", [&](const int val){ outputdamp = val;});
	pMapCv.emplace("outputdamp", [&](const int val){ cv_outputdamp = val;});
	pMapPar.emplace("outputdampbw", [&](const int val){ outputdampbw = val;});
	pMapCv.emplace("outputdampbw", [&](const int val){ cv_outputdampbw = val;});
	pMapPar.emplace("bassboost", [&](const int val){ bassboost = val;});
	pMapCv.emplace("bassboost", [&](const int val){ cv_bassboost = val;});
	pMapPar.emplace("damp2", [&](const int val){ damp2 = val;});
	pMapCv.emplace("damp2", [&](const int val){ cv_damp2 = val;});
	pMapPar.emplace("bassbw", [&](const int val){ bassbw = val;});
	pMapCv.emplace("bassbw", [&](const int val){ cv_bassbw = val;});
	pMapPar.emplace("exprtmod", [&](const int val){ exprtmod = val;});
	pMapTrig.emplace("exprtmod", [&](const int val){ trig_exprtmod = val;});
	pMapPar.emplace("spin", [&](const int val){ spin = val;});
	pMapCv.emplace("spin", [&](const int val){ cv_spin = val;});
	pMapPar.emplace("spinlimit", [&](const int val){ spinlimit = val;});
	pMapCv.emplace("spinlimit", [&](const int val){ cv_spinlimit = val;});
	pMapPar.emplace("wander", [&](const int val){ wander = val;});
	pMapCv.emplace("wander", [&](const int val){ cv_wander = val;});
	pMapPar.emplace("spin2", [&](const int val){ spin2 = val;});
	pMapCv.emplace("spin2", [&](const int val){ cv_spin2 = val;});
	pMapPar.emplace("spinlimit2", [&](const int val){ spinlimit2 = val;});
	pMapCv.emplace("spinlimit2", [&](const int val){ cv_spinlimit2 = val;});
	pMapPar.emplace("wander2", [&](const int val){ wander2 = val;});
	pMapCv.emplace("wander2", [&](const int val){ cv_wander2 = val;});
	pMapPar.emplace("spin2wander", [&](const int val){ spin2wander = val;});
	pMapCv.emplace("spin2wander", [&](const int val){ cv_spin2wander = val;});
	pMapPar.emplace("dry", [&](const int val){ dry = val;});
	pMapCv.emplace("dry", [&](const int val){ cv_dry = val;});
	pMapPar.emplace("wet", [&](const int val){ wet = val;});
	pMapCv.emplace("wet", [&](const int val){ cv_wet = val;});
	pMapPar.emplace("width", [&](const int val){ width = val;});
	pMapCv.emplace("width", [&](const int val){ cv_width = val;});
	pMapPar.emplace("mono", [&](const int val){ mono = val;});
	pMapTrig.emplace("mono", [&](const int val){ trig_mono = val;});
	isStereo = true;
	id = "GDVerb2";
	// sectionCpp0
}
