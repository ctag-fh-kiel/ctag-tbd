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
#include "esp_log.h"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

ctagSoundProcessorGDVerb::ctagSoundProcessorGDVerb() {
    isStereo = true;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
    updateParams = 1;
    strev.setSampleRate(44100.f);
    ESP_LOGE("GDVERB", "os fac %d, total fs %f, fs %f", strev.getOSFactor(), strev.getTotalSampleRate(), strev.getSampleRate());
}

void ctagSoundProcessorGDVerb::Process(const ProcessData &data) {
    if(updateParams){
        updateParams = 0;
        strev.setrt60((float)par_revtime / 4095.f * 20.f);
        strev.setdccutfreq((float)par_dccut);
        strev.setidiffusion1((float)par_idiffusion1 / 4095.f);
        strev.setidiffusion2((float)par_idiffusion2 / 4095.f);
        strev.setdiffusion1((float)par_diffusion1 / 4095.f);
        strev.setdiffusion2((float)par_diffusion1 / 4095.f);
        strev.setinputdamp((float) par_inputdamp);
        strev.setdamp((float)par_damp);
        strev.setoutputdamp((float)par_outputdamp);
        strev.setspin((float)par_spin / 4095.f * 10.f);
        strev.setspindiff((float)par_spindiff / 4095.f);
        strev.setspinlimit((float)par_spinlimit / 4095.f * 50.f);
        strev.setwander((float)par_wander / 4095.f);
        strev.setmodulationnoise1((float)par_modnoise1 / 4095.f * 0.1f);
        strev.setmodulationnoise2((float)par_modnoise2 / 4095.f * 0.1f);
        strev.setAutoDiff(par_autodiff);
        strev.setwetr((float)par_wet / 4095.f);
        strev.setdryr((float) par_dry / 4095.f);
        strev.setwidth((float) par_width / 4095.f);
        strev.setmono((bool)par_mono);
    }
    if(cv_par_width != -1){
        strev.setwidth(data.cv[cv_par_width] * data.cv[cv_par_width]);
    }
    if(cv_par_wet != -1){
        strev.setwetr(data.cv[cv_par_wet] * data.cv[cv_par_wet]);
    }
    if(cv_par_dry != -1){
        strev.setdryr(data.cv[cv_par_dry] * data.cv[cv_par_dry]);
    }
    if(cv_par_dccut != -1){
        strev.setdccutfreq(data.cv[cv_par_dccut] * data.cv[cv_par_dccut] * 10000.f);
    }
    if(cv_par_inputdamp != -1){
        strev.setinputdamp(data.cv[cv_par_inputdamp] * data.cv[cv_par_inputdamp] * 20000.f);
    }
    if(cv_par_damp != -1){
        strev.setdamp(data.cv[cv_par_damp] * data.cv[cv_par_damp] * 20000.f);
    }
    if(cv_par_outputdamp != -1){
        strev.setoutputdamp(data.cv[cv_par_outputdamp] * data.cv[cv_par_outputdamp] * 20000.f);
    }
    if(cv_par_wander != -1){
        strev.setwander(data.cv[cv_par_wander] * data.cv[cv_par_wander]);
    }

    // process the data
    strev.processreplace(data.buf, bufSz);
}

ctagSoundProcessorGDVerb::~ctagSoundProcessorGDVerb() {

}

const char *ctagSoundProcessorGDVerb::GetCStrID() const {
    return id.c_str();
}

void ctagSoundProcessorGDVerb::setParamValueInternal(const string &id, const string &key, const int val) {
    if (id.compare("revtime") == 0) {
        if (key.compare("current") == 0) {
            par_revtime = val;
            updateParams = 1;
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
    if (id.compare("dccut") == 0) {
        if (key.compare("current") == 0) {
            par_dccut = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_dccut = val;
            return;
        }
    }
    if (id.compare("idiffusion1") == 0) {
        if (key.compare("current") == 0) {
            par_idiffusion1 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_idiffusion1 = val;
            return;
        }
    }
    if (id.compare("idiffusion2") == 0) {
        if (key.compare("current") == 0) {
            par_idiffusion2 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_idiffusion2 = val;
            return;
        }
    }
    if (id.compare("diffusion1") == 0) {
        if (key.compare("current") == 0) {
            par_diffusion1 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_diffusion1 = val;
            return;
        }
    }
    if (id.compare("diffusion2") == 0) {
        if (key.compare("current") == 0) {
            par_diffusion2 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_diffusion2 = val;
            return;
        }
    }
    if (id.compare("inputdamp") == 0) {
        if (key.compare("current") == 0) {
            par_inputdamp = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_inputdamp = val;
            return;
        }
    }
    if (id.compare("damp") == 0) {
        if (key.compare("current") == 0) {
            par_damp = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_damp = val;
            return;
        }
    }
    if (id.compare("outputdamp") == 0) {
        if (key.compare("current") == 0) {
            par_outputdamp = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_outputdamp = val;
            return;
        }
    }
    if (id.compare("spin") == 0) {
        if (key.compare("current") == 0) {
            par_spin = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_spin = val;
            return;
        }
    }
    if (id.compare("spindiff") == 0) {
        if (key.compare("current") == 0) {
            par_spindiff = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_spindiff = val;
            return;
        }
    }
    if (id.compare("spinlimit") == 0) {
        if (key.compare("current") == 0) {
            par_spinlimit = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_spinlimit = val;
            return;
        }
    }
    if (id.compare("wander") == 0) {
        if (key.compare("current") == 0) {
            par_wander = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_wander = val;
            return;
        }
    }
    if (id.compare("modnoise1") == 0) {
        if (key.compare("current") == 0) {
            par_modnoise1 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_modnoise1 = val;
            return;
        }
    }
    if (id.compare("modnoise2") == 0) {
        if (key.compare("current") == 0) {
            par_modnoise2 = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_modnoise2 = val;
            return;
        }
    }
    if (id.compare("autodiff") == 0) {
        if (key.compare("current") == 0) {
            par_autodiff = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_autodiff = val;
            return;
        }
    }
    if (id.compare("wet") == 0) {
        if (key.compare("current") == 0) {
            par_wet = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_wet = val;
            return;
        }
    }
    if (id.compare("dry") == 0) {
        if (key.compare("current") == 0) {
            par_dry = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_dry = val;
            return;
        }
    }
    if (id.compare("width") == 0) {
        if (key.compare("current") == 0) {
            par_width = val;
            updateParams = 1;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_par_width = val;
            return;
        }
    }
    if (id.compare("mono") == 0) {
        if (key.compare("current") == 0) {
            par_mono = val;
            updateParams = 1;
            return;
        }
    }

}

void ctagSoundProcessorGDVerb::loadPresetInternal() {
    par_revtime = model->GetParamValue("revtime", "current");
    par_dccut = model->GetParamValue("dccut", "current");
    par_idiffusion1 = model->GetParamValue("idiffusion1", "current");
    par_idiffusion2 = model->GetParamValue("idiffusion2", "current");
    par_diffusion1 = model->GetParamValue("diffusion1", "current");
    par_diffusion2 = model->GetParamValue("diffusion2", "current");
    par_inputdamp = model->GetParamValue("inputdamp", "current");
    par_damp = model->GetParamValue("damp", "current");
    par_outputdamp = model->GetParamValue("outputdamp", "current");
    par_spin = model->GetParamValue("spin", "current");
    par_spindiff = model->GetParamValue("spindiff", "current");
    par_spinlimit = model->GetParamValue("spinlimit", "current");
    par_wander = model->GetParamValue("wander", "current");
    par_modnoise1 = model->GetParamValue("modnoise1", "current");
    par_modnoise2 = model->GetParamValue("modnoise2", "current");
    par_autodiff = model->GetParamValue("autodiff", "current");
    par_wet = model->GetParamValue("wet", "current");
    par_dry = model->GetParamValue("dry", "current");
    par_width = model->GetParamValue("width", "current");
    par_mono = model->GetParamValue("mono", "current");

    cv_par_revtime = model->GetParamValue("revtime", "cv");
    cv_par_dccut = model->GetParamValue("dccut", "cv");
    cv_par_idiffusion1 = model->GetParamValue("idiffusion1", "cv");
    cv_par_idiffusion2 = model->GetParamValue("idiffusion2", "cv");
    cv_par_diffusion1 = model->GetParamValue("diffusion1", "cv");
    cv_par_diffusion2 = model->GetParamValue("diffusion2", "cv");
    cv_par_inputdamp = model->GetParamValue("inputdamp", "cv");
    cv_par_damp = model->GetParamValue("damp", "cv");
    cv_par_outputdamp = model->GetParamValue("outputdamp", "cv");
    cv_par_spin = model->GetParamValue("spin", "cv");
    cv_par_spindiff = model->GetParamValue("spindiff", "cv");
    cv_par_spinlimit = model->GetParamValue("spinlimit", "cv");
    cv_par_wander = model->GetParamValue("drywet", "cv");
    cv_par_modnoise1 = model->GetParamValue("wander", "cv");
    cv_par_modnoise2 = model->GetParamValue("modnoise1", "cv");
    cv_par_autodiff = model->GetParamValue("modnoise2", "cv");
    cv_par_wet = model->GetParamValue("wet", "cv");
    cv_par_dry = model->GetParamValue("dry", "cv");
    cv_par_width = model->GetParamValue("width", "cv");
    updateParams = 1;
    strev.mute();
}
