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


#include "ctagSoundProcessorMonoMult.hpp"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

ctagSoundProcessorMonoMult::ctagSoundProcessorMonoMult()
{
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
}

void ctagSoundProcessorMonoMult::Process(const ProcessData &data){
    float deltaCV = (data.cv[cvControl] - preCV) / (float)bufSz; // for linear CV interpolation
    facDelayed = 0.995f * facDelayed + 0.005f * (float)factor / 1023.0f; // smooth parameter changes
    for(int i=0;i<this->bufSz;i++){ // iterate all channel samples
        data.buf[i*2 + this->processCh] *= facDelayed; // multiply with loudness factor
        if(cvControl != -1){
            data.buf[i*2 + this->processCh] *= preCV * preCV; // linearly interpolate CV data, square for loudness perception
            preCV += deltaCV;
        }
        if(enable != enableZ_1){ // switching from enabled to disabled or vice-versa with linear buffer smoothing
            enableZ_1 = enable;
            data.buf[i*2 + this->processCh] *= (float)(1 - enableZ_1) - ((float)i / (float)(bufSz - 1));
        }else if(enableZ_1 == 0){
            data.buf[i*2 + this->processCh] = 0.0f;
        }
        if(trigControl != -1){
            data.buf[i*2 + this->processCh] *= data.trig[trigControl];
        }
    }
}

ctagSoundProcessorMonoMult::~ctagSoundProcessorMonoMult(){
}

const char * ctagSoundProcessorMonoMult::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorMonoMult::setParamValueInternal(const string &id, const string &key, const int val) {
    ESP_LOGW("Monomult", "set param val id %s key %s val %d", id.c_str(), key.c_str(), val);
    if(id.compare("factor") == 0){
        if(key.compare("current") == 0){
            factor = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cvControl = val;
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
                trigControl = val;
            return;
        }
    }

}

void ctagSoundProcessorMonoMult::loadPresetInternal() {
    factor = model->GetParamValue("factor", "current");
    enable = model->GetParamValue("enable", "current");
    cvControl = model->GetParamValue("factor", "cv");
    trigControl = model->GetParamValue("enable", "trig");
    ESP_LOGW("Monomult", "Loaded factor %d, enable %d", factor.load(), enable.load());
}
