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


#include "ctagSoundProcessorDust.hpp"
#include <iostream>
#include <cmath>
#include "dsps_biquad_gen.h"
#include "dsps_biquad.h"
#include "helpers/ctagFastMath.hpp"


using namespace CTAG::SP;

ctagSoundProcessorDust::ctagSoundProcessorDust()
{
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
    dust.SetParams(0.5f, 1.0f, 0.0f, 44100.f);

    for(uint32_t i=0;i<6;i++){
        coeffs[i] = 1.f;
    }
    CTAG::SP::HELPERS::dsps_biquad_gen_bpf0db_f32(coeffs, 0.002, 0.4f);
    w1[0] = 0.f;
    w1[1] = 0.f;
    w2[0] = 0.f;
    w2[1] = 0.f;
}

void ctagSoundProcessorDust::Process(const ProcessData &data){
    float fRate = (float)rate / 100000.f;
    if(cvRate != -1){
        fRate = data.cv[cvRate];
    }
    float tmp[bufSz];
    fRate *= fRate;
    fRate *= 44100.f;
    dust.SetRate(fRate);
    dust.SetBipolar(bipolar);
    float fSmooth = (float)smooth / 4095.f;
    if(cvSmooth != -1){
        fSmooth = fabs(data.cv[cvSmooth]);
    }
    dust.SetSmooth(fSmooth);
    float fWidth = (float) width / 4095.f * 44100.f * 0.002f;
    if(cvWidth != -1){
        fWidth = fabs(data.cv[cvWidth] * 44100.f * 0.002f);
    }
    dust.SetWidth((uint32_t) fWidth);
    float fBPCut = (float)bp_fcut / 44100.f;
    if(cv_bp_fcut != -1){
        fBPCut = data.cv[cv_bp_fcut];
        fBPCut *= fBPCut;
        fBPCut *= 10000.f / 44100.f;
    }
    float fBPQ =  (float)bp_q / 4095.f * 100.f;
    if(cv_bp_q != -1){
        fBPQ = data.cv[cv_bp_q];
        fBPQ *= fBPQ;
        fBPQ *= 100.f;
    }
    CTAG::SP::HELPERS::dsps_biquad_gen_bpf0db_f32(coeffs, fBPCut, fBPQ);

    for(uint32_t i=0; i<bufSz; i++){
        tmp[i] = dust.Process();
    }
    if(bp_enable){
        if(trig_bp_enable != -1) {
            if(data.trig[trig_bp_enable] == 1)
                dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        }else{
            dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        }
    }else{
        if(trig_bp_enable != -1) {
            if(data.trig[trig_bp_enable] == 0)
                dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w1);
        }
    }

    // cascade
    //dsps_biquad_f32(tmp, tmp, bufSz, coeffs, w2);
    float fLevel = (float) level / 4095.f;
    if(cvLevel != -1){
        fLevel = data.cv[cvLevel];
    }
    fLevel *= fLevel;
    for(uint32_t i=0; i<bufSz; i++){
        data.buf[i*2 + processCh] = fLevel * tmp[i];
    }
}

ctagSoundProcessorDust::~ctagSoundProcessorDust(){
}

const char * ctagSoundProcessorDust::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorDust::setParamValueInternal(const string &id, const string &key, const int val) {
    if(id.compare("rate") == 0){
        if(key.compare("current") == 0){
            rate = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvRate = val;
            return;
        }
    }
    if(id.compare("level") == 0){
        if(key.compare("current") == 0){
            level = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvLevel = val;
            return;
        }
    }
    if(id.compare("smooth") == 0){
        if(key.compare("current") == 0){
            smooth = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvSmooth = val;
            return;
        }
    }
    if(id.compare("bipolar") == 0){
        if(key.compare("current") == 0){
            bipolar = val;
            return;
        }
    }
    if(id.compare("width") == 0){
        if(key.compare("current") == 0){
            width = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvWidth = val;
            return;
        }
    }
    if(id.compare("bp_enable") == 0){
        if(key.compare("current") == 0){
            bp_enable = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trig_bp_enable = val;
            return;
        }
    }
    if(id.compare("bp_fcut") == 0){
        if(key.compare("current") == 0){
            bp_fcut = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_bp_fcut = val;
            return;
        }
    }
    if(id.compare("bp_q") == 0){
        if(key.compare("current") == 0){
            bp_q = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_bp_q = val;
            return;
        }
    }
}

void ctagSoundProcessorDust::loadPresetInternal() {
    rate = model->GetParamValue("rate", "current");
    level = model->GetParamValue("level", "current");
    smooth = model->GetParamValue("smooth", "current");
    bipolar = model->GetParamValue("bipolar", "current");
    width = model->GetParamValue("width", "current");
    bp_enable = model->GetParamValue("bp_enable", "current");
    bp_q = model->GetParamValue("bp_q", "current");
    bp_fcut = model->GetParamValue("bp_fcut", "current");
    cvRate = model->GetParamValue("rate", "cv");
    cvLevel = model->GetParamValue("level", "cv");
    cvSmooth = model->GetParamValue("smooth", "cv");
    cvWidth = model->GetParamValue("width", "cv");
    cv_bp_fcut = model->GetParamValue("bp_fcut", "cv");
    cv_bp_q = model->GetParamValue("bp_q", "cv");
    trig_bp_enable = model->GetParamValue("bp_enable", "trig");
}
