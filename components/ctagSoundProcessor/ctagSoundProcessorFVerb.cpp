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

ctagSoundProcessorFVerb::ctagSoundProcessorFVerb()
{
    isStereo = true;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
}

void ctagSoundProcessorFVerb::Process(const ProcessData &data) {
    float val = (float)roomsz / 4095.f;
    if(cvRoomSz != -1) val = data.cv[cvRoomSz] * data.cv[cvRoomSz];
    freeverb.setroomsize(val);
    val = (float)damp / 4095.f;
    if(cvDamp != -1) val = data.cv[cvDamp] * data.cv[cvDamp];
    freeverb.setdamp(val);
    val = (float)dry / 4095.f;
    if(cvDry != -1) val = data.cv[cvDry] * data.cv[cvDry];
    freeverb.setdry(val);
    val = (float)width / 4095.f;
    if(cvWidth != -1) val = data.cv[cvWidth] * data.cv[cvWidth];
    freeverb.setwidth(val);
    val = (float)mode;
    if(trigMode != -1){
        val = 1.f - data.trig[trigMode];
    }
    freeverb.setmode(val);
    freeverb.setmono(mono);
    val = (float)wet / 4095.f;
    if(cvWet != -1) val = data.cv[cvWet] * data.cv[cvWet];
    freeverb.setwet(val);
    freeverb.process(data.buf, bufSz);
}

ctagSoundProcessorFVerb::~ctagSoundProcessorFVerb(){

}

const char * ctagSoundProcessorFVerb::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorFVerb::setParamValueInternal(const string id, const string key, const int val) {
    if(id.compare("roomsize") == 0){
        if(key.compare("current") == 0){
            roomsz = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvRoomSz = val;
            return;
        }
    }
    if(id.compare("damp") == 0){
        if(key.compare("current") == 0){
            damp = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvDamp = val;
            return;
        }
    }
    if(id.compare("dry") == 0){
        if(key.compare("current") == 0){
            dry = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvDry = val;
            return;
        }
    }
    if(id.compare("wet") == 0){
        if(key.compare("current") == 0){
            wet = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvWet = val;
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
    if(id.compare("mode") == 0){
        if(key.compare("current") == 0){
            mode = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trigMode = val;
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

void ctagSoundProcessorFVerb::loadPresetInternal() {
    roomsz = model->GetParamValue("roomsize", "current");
    damp = model->GetParamValue("damp", "current");
    dry = model->GetParamValue("dry", "current");
    wet = model->GetParamValue("wet", "current");
    width = model->GetParamValue("width", "current");
    mode = model->GetParamValue("mode", "current");
    mono = model->GetParamValue("mono", "current");
    cvRoomSz = model->GetParamValue("roomsize", "cv");
    cvDamp = model->GetParamValue("damp", "cv");
    cvDry = model->GetParamValue("dry", "cv");
    cvWet = model->GetParamValue("wet", "cv");
    cvWidth = model->GetParamValue("width", "cv");
    trigMode = model->GetParamValue("mode", "trig");
}
