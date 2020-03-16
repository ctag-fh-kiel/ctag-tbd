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


#include "ctagSoundProcessorPNoise.hpp"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

ctagSoundProcessorPNoise::ctagSoundProcessorPNoise()
{
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
}

void ctagSoundProcessorPNoise::Process(const ProcessData &data){
    fFactor = factor / (float)1023.f;
    if(cvControl != -1){
        fFactor = data.cv[cvControl];
        fFactor *= fFactor;
    }
    for(uint32_t i = 0;i<bufSz; i++){
        data.buf[i*2 + processCh] = noiseGen.Process() * fFactor * fFactor;
    }
}

ctagSoundProcessorPNoise::~ctagSoundProcessorPNoise(){
}

const char * ctagSoundProcessorPNoise::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorPNoise::setParamValueInternal(const string id, const string key, const int val) {
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

void ctagSoundProcessorPNoise::loadPresetInternal() {
    factor = model->GetParamValue("factor", "current");
    enable = model->GetParamValue("enable", "current");
    cvControl = model->GetParamValue("factor", "cv");
    trigControl = model->GetParamValue("enable", "trig");
}
