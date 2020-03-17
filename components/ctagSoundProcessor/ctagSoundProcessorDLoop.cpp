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


#include "ctagSoundProcessorDLoop.hpp"
#include "helpers/ctagFastMath.hpp"
#include <iostream>
#include <cmath>

using namespace CTAG::SP;

ctagSoundProcessorDLoop::ctagSoundProcessorDLoop()
{
    isStereo = true;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
    // init params
    d.SetBipolar(false);
    d.ReSeed(seed);
    d_pan.SetBipolar( true);
    d_pan.ReSeed(seed + ofssspread);
    d_level.SetBipolar( false);
    d_level.ReSeed(seed + ofsvspread);
    s_l.SetBipolar(false);
    s_l.ReSeed(seed + 1234);
    s_r.SetBipolar(false);
    s_r.ReSeed(seed + 5678);

    loopCntr = 0;
    lastReset = 0;
    dccutl.setCutOnFreq(20, 44100.f);
    dccutr.setCutOnFreq(20, 44100.f);
    decL.SetSampleRate(44100.f);
    decL.SetCoeffSmoothing(false);
    decR.SetSampleRate(44100.f);
    decR.SetCoeffSmoothing(false);

}

void ctagSoundProcessorDLoop::Process(const ProcessData &data){
    // params and CV / trig mods
    float thres = (float)density / 4095.f;
    if(cv_density != -1) thres = data.cv[cv_density];
    thres *= thres;
    thres = 1.f - thres;
    float flevel = (float)level / 4095.f;
    if(cv_level != -1) flevel = data.cv[cv_level];
    flevel *= flevel;
    flevel *= 2.f;
    float fsspread = (float)sspread / 4095.f; // stereo spread
    if(cv_sspread != -1) fsspread = data.cv[cv_sspread] * data.cv[cv_sspread];
    float fvspread = (float)vspread / 4095.f;
    if(cv_vspread != -1) fvspread = data.cv[cv_vspread] * data.cv[cv_vspread];
    fvspread *= -100.f; // volume variance
    uint32_t isLoop = loop;
    if(t_loop != -1) isLoop = data.trig[t_loop] == 1 ? 0 : 1;
    uint32_t sequenceLength = slen;
    if(cv_slen != -1) sequenceLength = (uint32_t )(data.cv[cv_slen] * data.cv[cv_slen] * 100000.f);
    uint32_t sequenceSeed = seed;
    if(cv_seed != -1) sequenceSeed = (uint32_t)(data.cv[cv_seed] * data.cv[cv_seed] * 1024.f); // limit range for more stability due to adc noise
    uint32_t  isReset = reset;
    if(t_reset != -1) isReset = data.trig[t_reset] == 1 ? 0 : 1;
    uint32_t isRandomShaping = s_enable;// == 1 ? 0 : 1;
    if(t_s_enable != -1) isRandomShaping = data.trig[t_s_enable] == 1 ? 0 : 1;
    float decay = (float) s_decay / 4095.f * 0.05f;
    if(cv_s_decay != -1) decay = data.cv[cv_s_decay] * data.cv[cv_s_decay] * 0.05f;
    decL.SetDecay60dB(decay);
    decR.SetCoeff(decL.GetCoeff());
    float rShape = (float)s_rlevel / 4095.f;
    if(cv_s_rlevel != -1) rShape = data.cv[cv_s_rlevel] * data.cv[cv_s_rlevel];

    // create audio buffer
    if(loopCntr < sequenceLength){
        for(uint32_t i=0; i<bufSz; i++){
            float val = d.Process();
            val = val > thres ? 1.0f : 0.f; // dust grain
            float l = d_level.Process(); // random
            l *= fvspread;
            val *= fast_VdB(l);
            val *= flevel;
            float pan = d_pan.Process() * 0.5f;
            float t = val * (0.5f + fsspread * pan);
            if(t>1.f) t = 1.f;
            if(isRandomShaping && t > 0.001f){
                float a = 4.f * rShape * s_l.Process();
                if(a>1.f)a = 0.99999999999999999f;
                decL.SetCoeff(a);
            }
            t = decL.Process(t);
            data.buf[i*2] = dccutl.process(t); // left minus dc offset
            t = val * (0.5f - fsspread * pan);
            if(t>1.f) t = 1.f;
            if(isRandomShaping && t > 0.001f){
                float a = 4.f * rShape * s_r.Process();
                if(a>1.f)a = 0.99999999999999999f;
                decR.SetCoeff(a);
            }
            t = decR.Process(t);

            data.buf[i*2 + 1] = dccutr.process(t);// right minus dc offset
        }
    }

    // end of loop? restart
    uint32_t ofsStereoSpread = ofssspread;
    if(cv_ofssspread != -1) ofsStereoSpread = data.cv[cv_ofssspread] * 10000.f;
    uint32_t ofsVolumeSpread = ofsvspread;
    if(cv_ofsvspread != -1) ofsVolumeSpread = data.cv[cv_ofsvspread] * 10000.f;
    if((loopCntr >= sequenceLength && isLoop == 1) || (isReset == 1 && lastReset == 0)){
        loopCntr = 0;
        d.ReSeed(sequenceSeed);
        d_pan.ReSeed(sequenceSeed + ofsStereoSpread);
        d_level.ReSeed(sequenceSeed + ofsVolumeSpread);
        s_l.ReSeed(sequenceSeed + 1234);
        s_r.ReSeed(sequenceSeed + 5678);
    }else{
        loopCntr++;
    }
    lastReset = isReset;
}

ctagSoundProcessorDLoop::~ctagSoundProcessorDLoop(){
}

const char * ctagSoundProcessorDLoop::GetCStrID() const{
    return id.c_str();
}

void ctagSoundProcessorDLoop::setParamValueInternal(const string &id, const string &key, const int val) {
    if(id.compare("reset") == 0){
        if(key.compare("current") == 0){
            reset = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                t_reset = val;
            return;
        }
    }
    if(id.compare("loop") == 0){
        if(key.compare("current") == 0){
            loop = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                t_loop = val;
            return;
        }
    }
    if(id.compare("seed") == 0){
        if(key.compare("current") == 0){
            seed = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_seed = val;
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
                cv_level = val;
            return;
        }
    }
    if(id.compare("density") == 0){
        if(key.compare("current") == 0){
            density = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_density = val;
            return;
        }
    }
    if(id.compare("slen") == 0){
        if(key.compare("current") == 0){
            slen = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_slen = val;
            return;
        }
    }
    if(id.compare("sspread") == 0){
        if(key.compare("current") == 0){
            sspread = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_sspread = val;
            return;
        }
    }
    if(id.compare("ofssspread") == 0){
        if(key.compare("current") == 0){
            ofssspread = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_ofssspread = val;
            return;
        }
    }
    if(id.compare("vspread") == 0){
        if(key.compare("current") == 0){
            vspread = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_vspread = val;
            return;
        }
    }
    if(id.compare("ofsvspread") == 0){
        if(key.compare("current") == 0){
            ofsvspread = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_ofsvspread = val;
            return;
        }
    }
    if(id.compare("s_enable") == 0){
        if(key.compare("current") == 0){
            s_enable = val;
            return;
        }
        if(key.compare("trig") == 0){
            if(val >= -1 && val <= 1)
                t_s_enable = val;
            return;
        }
    }
    if(id.compare("s_rlevel") == 0){
        if(key.compare("current") == 0){
            s_rlevel = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_s_rlevel = val;
            return;
        }
    }
    if(id.compare("s_decay") == 0){
        if(key.compare("current") == 0){
            s_decay = val;
            return;
        }
        if(key.compare("cv") == 0){
            if(val >= -1 && val <= 3)
                cv_s_decay = val;
            return;
        }
    }
}

void ctagSoundProcessorDLoop::loadPresetInternal() {
    reset = model->GetParamValue("reset", "current");
    t_reset = model->GetParamValue("reset", "trig");
    loop = model->GetParamValue("loop", "current");
    t_loop = model->GetParamValue("loop", "trig");
    seed = model->GetParamValue("seed", "current");
    cv_seed = model->GetParamValue("seed", "cv");
    level = model->GetParamValue("level", "current");
    cv_level = model->GetParamValue("level", "cv");
    density = model->GetParamValue("density", "current");
    cv_density = model->GetParamValue("density", "cv");
    slen = model->GetParamValue("slen", "current");
    cv_slen = model->GetParamValue("slen", "cv");
    sspread = model->GetParamValue("sspread", "current");
    cv_sspread = model->GetParamValue("sspread", "cv");
    ofssspread = model->GetParamValue("ofssspread", "current");
    cv_ofssspread = model->GetParamValue("ofssspread", "cv");
    vspread = model->GetParamValue("vspread", "current");
    cv_vspread = model->GetParamValue("vspread", "cv");
    ofsvspread = model->GetParamValue("ofsvspread", "current");
    cv_ofsvspread = model->GetParamValue("ofsvspread", "cv");
    s_enable = model->GetParamValue("s_enable", "current");
    t_s_enable = model->GetParamValue("s_enable", "trig");
    s_rlevel = model->GetParamValue("ofssspread", "current");
    cv_s_rlevel = model->GetParamValue("ofssspread", "cv");
    s_decay = model->GetParamValue("s_decay", "current");
    cv_s_decay = model->GetParamValue("s_decay", "cv");
}
