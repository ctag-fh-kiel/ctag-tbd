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


#include "ctagSoundProcessorFBDlyLine.hpp"
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP;

ctagSoundProcessorFBDlyLine::ctagSoundProcessorFBDlyLine() :
    maxLength(88200),
    dlyLine(maxLength)
{
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
}

void ctagSoundProcessorFBDlyLine::Process(const ProcessData &data){
    if(trigControlEnable != -1){
        if(data.trig[trigControlEnable] == 1 && enable == 1) return;
        else if(data.trig[trigControlEnable] == 0 && enable == 0) return;
    }else{
        if(enable == 0) return;
    }
    float fb = feedback / 4095.f;
    if(cvControlFeedback != -1){
        fb += data.cv[cvControlFeedback] - 0.5f;
        if(fb > 1.f) fb = 1.f;
        if(fb < 0.f) fb = 0.f;
    }
    cvLength = (float)length;
    if(cvControlLength != -1){
        cvLength += (data.cv[cvControlLength] - 0.5) * (float)(maxLength / 2);
    }
    fLength = 0.5f * fLength + 0.5f * cvLength;
    if(fLength > maxLength) fLength = maxLength;
    if(fLength < 1) fLength = 1.f;
    dlyLine.SetLength((uint32_t) fLength);
    dlyLine.SetFeedback(fb);
    dlyLine.Process(data.buf, this->processCh, 2, 32*2);

    fLevel = (float)level / 4095.f;
    if(cvControlLevel != -1){
        fLevel += data.cv[cvControlLevel] - 0.5f;
        if(fLevel < 0.f) fLevel = 0.f;
        if(fLevel > 1.f) fLevel = 1.f;
    }
    fDryWet = (float)drywet / 4095.f;
    for(uint32_t i = 0; i<bufSz; i++){
        data.buf[i*2 + processCh] *= fLevel;
    }
}

ctagSoundProcessorFBDlyLine::~ctagSoundProcessorFBDlyLine(){
}

const char * ctagSoundProcessorFBDlyLine::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorFBDlyLine::setParamValueInternal(const string &id, const string &key, const int val) {
    if(id.compare("length") == 0){
        if(key.compare("current") == 0){
            length = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlLength = val;
            return;
        }
    }
    if(id.compare("feedback") == 0){
        if(key.compare("current") == 0){
            feedback = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlFeedback = val;
            return;
        }
    }
    if(id.compare("drywet") == 0){
        if(key.compare("current") == 0){
            drywet = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControlDryWet = val;
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
                cvControlLevel = val;
            return;
        }
    }
    if(id.compare("enable") == 0){
        if(key.compare("current") == 0){
            enable = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                trigControlEnable = val;
            return;
        }
    }
}

void ctagSoundProcessorFBDlyLine::loadPresetInternal() {
    feedback = model->GetParamValue("feedback", "current");
    length = model->GetParamValue("length", "current");
    drywet = model->GetParamValue("drywet", "current");
    level = model->GetParamValue("level", "current");
    enable = model->GetParamValue("enable", "current");
    cvControlLength = model->GetParamValue("length", "cv");
    cvControlFeedback = model->GetParamValue("feedback", "cv");
    cvControlDryWet = model->GetParamValue("drywet", "cv");
    cvControlLevel = model->GetParamValue("level", "cv");
    trigControlEnable = model->GetParamValue("enable", "trig");
}
