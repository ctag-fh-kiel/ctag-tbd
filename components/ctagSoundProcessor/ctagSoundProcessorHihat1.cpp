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


#include "ctagSoundProcessorHihat1.hpp"
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"


using namespace CTAG::SP;

ctagSoundProcessorHihat1::ctagSoundProcessorHihat1()
{
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();

    // init params
    wNoise.SetBipolar(true);
    filterBP.SetType(HELPERS::BIQUAD_TYPE::BP);
    filterBP.SetQ(0.75f);
    // ad envs
    adEnv.SetSampleRate(44100.f);
    adEnv.SetModeExp();
    pitchEnv.SetSampleRate(44100.f / (float) bufSz);
    pitchEnv.SetModeLin();
}

void ctagSoundProcessorHihat1::Process(const ProcessData &data){
    float deltaCVLoud = (data.cv[cvControlLoudness] - preCVLoudness) / (float)bufSz; // for linear CV interpolation
    freq = (float) frequency; // frequency
    // cv frequency
    if(cvControlFrequency != -1){
        preCVFrequency = 0.3f * data.cv[cvControlFrequency] + 0.7f * preCVFrequency; // smooth CV
        float fMod = preCVFrequency * 5.f;
        fMod = CTAG::SP::HELPERS::fastpow2(fMod);
        freq *= fMod;
    }
    // eg pitch
    if(enableEG_p == 1 && trigEG_p != -1){
        if(data.trig[trigEG_p] != prevTrigState_p){
            prevTrigState_p = data.trig[trigEG_p];
            if(prevTrigState_p == 0) pitchEnv.Trigger();
        }
        pitchEnv.SetLoop(loopEG_p);
        attackVal_p = (float)attack_p / 4095.f;
        if(cvControlAttack_p != -1){
            attackVal_p = data.cv[cvControlAttack_p] * data.cv[cvControlAttack_p];
        }
        decayVal_p = (float)decay_p / 4095.f;
        if(cvControlDecay_p != -1){
            decayVal_p = data.cv[cvControlDecay_p] * data.cv[cvControlDecay_p];
        }
        pitchEnv.SetAttack(attackVal_p);
        pitchEnv.SetDecay(decayVal_p);
    }
    // parameter control
    loud = (float)loudness / 4095.f;
    //  eg loud
    if(enableEG == 1 && trigEG != -1){
        if(data.trig[trigEG] != prevTrigState){
            prevTrigState = data.trig[trigEG];
            if(prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        attackVal = (float)attack / 4095.f;
        if(cvControlAttack != -1){
            attackVal = data.cv[cvControlAttack] * data.cv[cvControlAttack];
        }
        decayVal = (float)decay / 4095.f;
        if(cvControlDecay != -1){
            decayVal = data.cv[cvControlDecay] * data.cv[cvControlDecay];
        }
        adEnv.SetAttack(attackVal);
        adEnv.SetDecay(decayVal);
    }
    // here is the oscillator
    float freqb = freq;
    float tmp[bufSz];
    int nt = ntype;
    if(trigNType != -1) nt = data.trig[trigNType];
    for(int i=0;i<bufSz;i++){
        if(nt == 0)
            tmp[i] = wNoise.Process();
        else if(nt == 1)
            tmp[i] = pNoise.Process();
    }
    if(enableEG_p == 1 && trigEG_p != -1){
        float val;
        if(cvAmount_p == -1)
            val = CTAG::SP::HELPERS::fasterpow2(pitchEnv.Process() * (float)amount_p / 4095.f * 8.f);
        else
            val = CTAG::SP::HELPERS::fasterpow2(pitchEnv.Process() * data.cv[cvAmount_p] * 8.f);
        freqb *= val;
        if(freqb > 20000.f) freqb = 20000.f;
        if(freqb < 15.f) freqb = 15.f;
    }
    float q = (float)qfac / 4095.f * 100.f;
    if(cvControlQFac != -1){
        q = data.cv[cvControlQFac];
        q *= q;
        q *= 100.f;
    }
    filterBP.SetQ(q);
    filterBP.SetCutoffHz(freqb);
    filterBP.Process(tmp, bufSz);
    // apply loudness
    for(int i=0;i<this->bufSz;i++){ // iterate all channel samples
        data.buf[i*2 + this->processCh] = tmp[i] * loud;
        // apply loud EG
        if(enableEG == 1){
            data.buf[i*2 + this->processCh] *= adEnv.Process();
        }
        // apply CV loud control
        if(cvControlLoudness != -1){
            data.buf[i*2 + this->processCh] *= preCVLoudness * preCVLoudness; // linearly interpolate CV data, square for loudness perception
            preCVLoudness += deltaCVLoud;
        }
    }
}

ctagSoundProcessorHihat1::~ctagSoundProcessorHihat1(){
}

const char * ctagSoundProcessorHihat1::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorHihat1::setParamValueInternal(const string &id, const string &key, const int val) {
    if(id.compare("ntype") == 0){
        if(key.compare("current") == 0){
            ntype = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trigNType = val;
            return;
        }
    }
    if(id.compare("loudness") == 0){
        if(key.compare("current") == 0){
            loudness = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlLoudness = val;
            return;
        }
    }
    if(id.compare("qfac") == 0){
        if(key.compare("current") == 0){
            qfac = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlQFac = val;
            return;
        }
    }
    if(id.compare("frequency") == 0){
        if(key.compare("current") == 0){
            frequency = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlFrequency = val;
            return;
        }
    }
    if(id.compare("enableEG") == 0){
        if(key.compare("current") == 0){
            enableEG = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trigEG = val;
            return;
        }
    }
    if(id.compare("loopEG") == 0){
        if(key.compare("current") == 0){
            loopEG = val;
            return;
        }
    }
    if(id.compare("decay") == 0){
        if(key.compare("current") == 0){
            decay = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlDecay = val;
            return;
        }
    }
    if(id.compare("attack") == 0){
        if(key.compare("current") == 0){
            attack = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlAttack = val;
            return;
        }
    }
    if(id.compare("enableEG_p") == 0){
        if(key.compare("current") == 0){
            enableEG_p = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trigEG_p = val;
            return;
        }
    }
    if(id.compare("loopEG_p") == 0){
        if(key.compare("current") == 0){
            loopEG_p = val;
            return;
        }
    }
    if(id.compare("decay_p") == 0){
        if(key.compare("current") == 0){
            decay_p = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlDecay_p = val;
            return;
        }
    }
    if(id.compare("attack_p") == 0){
        if(key.compare("current") == 0){
            attack_p = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlAttack_p = val;
            return;
        }
    }
    if(id.compare("amount_p") == 0){
        if(key.compare("current") == 0){
            amount_p = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvAmount_p = val;
            return;
        }
    }
}

void ctagSoundProcessorHihat1::loadPresetInternal() {
    ntype = model->GetParamValue("ntype", "current");
    frequency = model->GetParamValue("frequency", "current");
    loudness = model->GetParamValue("loudness", "current");
    qfac = model->GetParamValue("qfac", "current");
    enableEG = model->GetParamValue("enableEG", "current");
    loopEG = model->GetParamValue("loopEG", "current");
    attack = model->GetParamValue("attack", "current");
    decay = model->GetParamValue("decay", "current");
    enableEG_p = model->GetParamValue("enableEG_p", "current");
    loopEG_p = model->GetParamValue("loopEG_p", "current");
    attack_p = model->GetParamValue("attack_p", "current");
    decay_p = model->GetParamValue("decay_p", "current");
    amount_p = model->GetParamValue("amount_p", "current");

    trigNType = model->GetParamValue("ntype", "trig");
    cvControlLoudness = model->GetParamValue("loudness", "cv");
    cvControlQFac = model->GetParamValue("qfac", "cv");
    cvControlFrequency = model->GetParamValue("frequency", "cv");
    cvControlAttack = model->GetParamValue("attack", "cv");
    cvControlDecay = model->GetParamValue("decay", "cv");
    trigEG = model->GetParamValue("enableEG", "trig");
    cvControlAttack_p = model->GetParamValue("attack_p", "cv");
    cvControlDecay_p = model->GetParamValue("decay_p", "cv");
    trigEG_p = model->GetParamValue("enableEG_p", "trig");
    cvAmount_p = model->GetParamValue("amount_p", "cv");
}
