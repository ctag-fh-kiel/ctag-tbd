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

using namespace CTAG::SP;

ctagSoundProcessorGVerb::ctagSoundProcessorGVerb()
{
    isStereo = true;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
    maxRoomSize = 500.f;
    gverb = (ty_gverb*)malloc(sizeof(ty_gverb));
    if(gverb == nullptr){
        ESP_LOGE("GVERB", "Fatal error, couldn't allocate memory!");
    }
                                    // roomSz, time, damp, spread, ibw, early, tail
    gverb_new(gverb, 44100, maxRoomSize, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
}

void ctagSoundProcessorGVerb::Process(const ProcessData &data) {
    float revout_l, revout_r;
    fRoomSz = (float)roomSz / 4095.f * maxRoomSize;
    fRevTime = (float)revTime / 4095.f * 30.f;
    fDamping = (float)damping / 4095.f - 0.01f;
    fInputBW = (float)inputBW / 4095.f;
    fEarlyLvl = (float)earlyLvl / 4095.f;
    fTailLvl = (float)tailLvl / 4095.f;
    fDryWet = (float)dryWet / 4095.f;
//    if(cvRoomSz != -1) fRoomSz = data.cv[cvRoomSz] * data.cv[cvRoomSz] * maxRoomSize;
    gverb_set_roomsize(gverb, fRoomSz);
    if(cvRevTime != -1) fRevTime = data.cv[cvRevTime] * data.cv[cvRevTime] * 30.f;
    gverb_set_revtime(gverb, fRevTime);
    if(cvDamping != -1) fDamping = (data.cv[cvDamping] * data.cv[cvDamping]) - 0.01f;
    gverb_set_damping(gverb, fDamping);
    if(cvInputBW != -1) fInputBW = data.cv[cvInputBW] * data.cv[cvInputBW];
    gverb_set_inputbandwidth(gverb, fInputBW);
    if(cvEarlyLvl != -1) fEarlyLvl = data.cv[cvEarlyLvl] * data.cv[cvEarlyLvl];
    gverb_set_earlylevel(gverb, fEarlyLvl);
    if(cvTailLvl != -1) fTailLvl = data.cv[cvTailLvl] * data.cv[cvTailLvl];
    gverb_set_taillevel(gverb, fTailLvl);
    if(cvDryWet != -1) fDryWet = data.cv[cvDryWet] * data.cv[cvDryWet];
    for(int i=0;i<bufSz;i++){
        if(mono){
            gverb_do(gverb, data.buf[i*2], &revout_l, &revout_r);
        }else{
            gverb_do(gverb, (data.buf[i*2] + data.buf[i*2 + 1])*0.5f, &revout_l, &revout_r);
        }
        data.buf[i*2] = fDryWet * revout_l + (1.f - fDryWet) * data.buf[i*2];
        if(mono){
            data.buf[i*2 + 1] = fDryWet * revout_r + (1.f - fDryWet) * data.buf[i*2];
        }else{
            data.buf[i*2 + 1] = fDryWet * revout_r + (1.f - fDryWet) * data.buf[i*2 + 1];
        }
    }
}

ctagSoundProcessorGVerb::~ctagSoundProcessorGVerb(){
    // careful, this also frees itself
    gverb_free(gverb);
}

const char * ctagSoundProcessorGVerb::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorGVerb::setParamValueInternal(const string &id, const string &key, const int val) {
    if(id.compare("roomsize") == 0){
        if(key.compare("current") == 0){
            roomSz = val;
            return;
        }
        /*
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvRoomSz = val;
            return;
        }
         */
    }
    if(id.compare("revtime") == 0){
        if(key.compare("current") == 0){
            revTime = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvRevTime = val;
            return;
        }
    }
    if(id.compare("damping") == 0){
        if(key.compare("current") == 0){
            damping = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvDamping = val;
            return;
        }
    }
    if(id.compare("inputbw") == 0){
        if(key.compare("current") == 0){
            inputBW = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvInputBW = val;
            return;
        }
    }
    if(id.compare("earlylvl") == 0){
        if(key.compare("current") == 0){
            earlyLvl = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvEarlyLvl = val;
            return;
        }
    }
    if(id.compare("taillvl") == 0){
        if(key.compare("current") == 0){
            tailLvl = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvTailLvl = val;
            return;
        }
    }
    if(id.compare("drywet") == 0){
        if(key.compare("current") == 0){
            dryWet = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvDryWet = val;
            return;
        }
    }
    if (id.compare("mono") == 0) {
        if (key.compare("current") == 0) {
            mono = val;
            return;
        }
    }
}

void ctagSoundProcessorGVerb::loadPresetInternal() {
    roomSz = model->GetParamValue("roomsize", "current");
    revTime = model->GetParamValue("revtime", "current");
    damping = model->GetParamValue("damping", "current");
    inputBW = model->GetParamValue("inputbw", "current");
    earlyLvl = model->GetParamValue("earlylvl", "current");
    tailLvl = model->GetParamValue("taillvl", "current");
    dryWet = model->GetParamValue("drywet", "current");
    mono = model->GetParamValue("mono", "current");
    //cvRoomSz = model->GetParamValue("roomsize", "cv");
    cvRevTime = model->GetParamValue("revtime", "cv");
    cvDamping = model->GetParamValue("damping", "cv");
    cvInputBW = model->GetParamValue("inputbw", "cv");
    cvEarlyLvl = model->GetParamValue("earlylvl", "cv");
    cvTailLvl = model->GetParamValue("taillvl", "cv");
    cvDryWet = model->GetParamValue("drywet", "cv");
}
