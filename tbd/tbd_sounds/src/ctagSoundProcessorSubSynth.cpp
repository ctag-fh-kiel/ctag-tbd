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


#include <tbd/sounds/ctagSoundProcessorSubSynth.hpp>
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"
#include "dsps_biquad.h"
#include "dsps_add.h"
#include "dsps_mul.h"
#include "dsps_mulc.h"
#include "dsps_addc.h"

#ifndef TBD_SIM
#include "xtensa/core-macros.h"
#endif

using namespace std;
using namespace CTAG::SP;

void ctagSoundProcessorSubSynth::Init(std::size_t blockSize, void *blockPtr) {
    isStereo = false;
    // acquire model from spiffs json, model auto loads last active preset
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    // take preset values from model
    loadPresetInternal();
    // inits
    memset(filterZs, 0, 10 * 3 * 2 * sizeof(float));
    memset(fCoeffs, 0, 6 * sizeof(float));
    wNoise.SetBipolar(true);
#ifndef TBD_SIM
    wNoise.ReSeed(XTHAL_GET_CCOUNT()); // seed random from CPU time ticks
    pNoise.ReSeed(XTHAL_GET_CCOUNT());
#endif
    adsrEnvSum.SetSampleRate(44100.f);
    adsrEnvSum.SetModeExp();
    eg[0].SetModeLin();
    eg[0].SetSampleRate(44100.f);
    eg[1].SetModeLin();
    eg[1].SetSampleRate(44100.f);
    prevTrigStateEG[0] = prevTrigStateEG[1] = 1;
    prevMode[0] = prevMode[1] = 0;

}

void ctagSoundProcessorSubSynth::Process(const ProcessData &data) {
    fetchControlData(data);
    updateEGs(data);
    float tmpIn[bufSz], tmpOut[bufSz], acc[bufSz];
    float egVals[2][bufSz], egApply[bufSz];
    // create white noise, pink noise or use external signal
    for (uint32_t i = 0; i < bufSz; i++) {
        switch (srcsel.load()) {
            case 0:
                tmpIn[i] = wNoise.Process();
                break;
            case 1:
                tmpIn[i] = pNoise.Process();
                break;
            case 2:
                tmpIn[i] = data.buf[i * 2 + processCh];
                break;
            default:
                break;
        }

        egVals[0][i] = eg[0].Process();
        egVals[1][i] = eg[1].Process();
    }
    // calculate filter coeffs and apply filters
    // root
    computeFilterCoefs(fCoeffs, fRootFrequency, fRootBWidth, fRootLevel * computeRolloff(fRootFrequency));
    int32_t cMax = cascade;
    for (uint32_t c = 0; c < cMax; c++) {
        if (c == 0) dsps_biquad_f32(tmpIn, acc, bufSz, fCoeffs, filterZs[c]);
        else
            dsps_biquad_f32(acc, acc, bufSz, fCoeffs, filterZs[c]);
    }

    // partials
    uint32_t pMax = partials;
    for (uint32_t p = 0; p < pMax; p++) {
        float freq = fRootFrequency * fHarm[p];
        //if (freq > 20000.f || freq < 20.f) continue;
        float bw = fBWidth[p] + fModBW[p] * egVals[p_modbwsrc[p]][bufSz - 1];
        if (bw > 1.f) bw = 1.f;
        if (bw < 0.f) bw = 0.f;
        computeFilterCoefs(fCoeffs, freq, bw, fGain[p] * computeRolloff(freq));
        for (uint32_t c = 0; c < cMax; c++) {
            if (c == 0) dsps_biquad_f32(tmpIn, tmpOut, bufSz, fCoeffs, filterZs[(p + 1) * 3]);
            else
                dsps_biquad_f32(tmpOut, tmpOut, bufSz, fCoeffs, filterZs[(p + 1) * 3 + c]);
        }
        // apply loudness envelope
        dsps_mulc_f32(egVals[p_modgainsrc[p]], egApply, bufSz, fModGain[p], 1, 1);
        dsps_addc_f32(egApply, egApply, bufSz, (1.f - fModGain[p]), 1, 1);
        dsps_mul_f32(tmpOut, egApply, tmpOut, bufSz, 1, 1, 1);
        // accumulate buffers
        dsps_add_f32(tmpOut, acc, acc, bufSz, 1, 1, 1);
    }

    // re-arrange data array and apply sum eg
    for (uint32_t i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] = acc[i] * fSumGain;
        // apply loud EG
        if (enableEG == 1) {
            data.buf[i * 2 + processCh] *= adsrEnvSum.Process();
        }
        data.buf[i * 2 + processCh] = HELPERS::fasttanh(data.buf[i * 2 + processCh]);
    }
}


void ctagSoundProcessorSubSynth::updateEGs(const ProcessData &data) {
    //  eg loud sum
    if (enableEG == 1 && trig_enableEG != -1) {
        adsrEnvSum.Gate(data.trig[trig_enableEG] == 1 ? false : true);
        float attackVal = (float) attack / 4095.f * 10.f;
        if (cv_attack != -1) {
            attackVal = data.cv[cv_attack] * data.cv[cv_attack];
        }
        float decayVal = (float) decay / 4095.f * 20.f;
        if (cv_decay != -1) {
            decayVal = data.cv[cv_decay] * data.cv[cv_decay];
        }
        float sustainVal = (float) sustain / 4095.f;
        if (cv_sustain != -1) {
            sustainVal = data.cv[cv_sustain] * data.cv[cv_sustain];
        }
        float releaseVal = (float) release / 4095.f * 20.f;
        if (cv_release != -1) {
            releaseVal = data.cv[cv_release] * data.cv[cv_release];
        }
        adsrEnvSum.SetAttack(attackVal);
        adsrEnvSum.SetDecay(decayVal);
        adsrEnvSum.SetSustain(sustainVal);
        adsrEnvSum.SetRelease(releaseVal);
    }
    // free egs setup
    for (int i = 0; i < 2; i++) {
        if (prevMode[i] != eg_mode[i]) {
            if (eg_mode[i] == 0) eg[i].SetModeLin();
            else eg[i].SetModeExp();
            prevMode[i] = eg_mode[i];
        }
        if (trig_eg_loopEG[i] == -1) {
            eg[i].SetLoop(eg_loopEG[i]);
        } else {
            eg[i].SetLoop(data.trig[trig_eg_loopEG[i]] == 1 ? false : true);
        }
        if (eg_enableEG[i] == 1 && trig_eg_enableEG[i] != -1) {
            if (data.trig[trig_eg_enableEG[i]] != prevTrigStateEG[i]) {
                prevTrigStateEG[i] = data.trig[trig_eg_enableEG[i]];
                if (prevTrigStateEG[i] == 0) eg[i].Trigger();
            }
        } else if (eg_enableEG[i] == 0) { // free running
            if (eg[i].GetLoop() == 1 && eg[i].GetIsRunning() == 0) eg[i].Trigger();
        }
        float attackVal = (float) eg_attack[i] / 4095.f * 10.f;
        if (cv_eg_attack[i] != -1) {
            attackVal = data.cv[cv_eg_attack[i]] * data.cv[cv_eg_attack[i]];
        }
        float decayVal = (float) eg_decay[i] / 4095.f * 10.f;
        if (cv_eg_decay[i] != -1) {
            decayVal = data.cv[cv_eg_decay[i]] * data.cv[cv_eg_decay[i]];
        }
        //TBD_LOGE("ad", "eg %d, Attackval %f, decayval %f", i, attackVal, decayVal);
        eg[i].SetAttack(attackVal);
        eg[i].SetDecay(decayVal);
    }
}

const char *ctagSoundProcessorSubSynth::GetCStrID() const {
    return id.c_str();
}

void ctagSoundProcessorSubSynth::setParamValueInternal(const string &id, const string &key, const int val) {
// autogenerated code here
// sectionCpp0
    if (id.compare("srcsel") == 0) {
        if (key.compare("current") == 0) {
            srcsel = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_srcsel = val;
            return;
        }
    }
    if (id.compare("cascade") == 0) {
        if (key.compare("current") == 0) {
            cascade = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_cascade = val;
            return;
        }
    }
    if (id.compare("partials") == 0) {
        if (key.compare("current") == 0) {
            partials = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_partials = val;
            return;
        }
    }
    if (id.compare("gain") == 0) {
        if (key.compare("current") == 0) {
            gain = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_gain = val;
            return;
        }
    }
    if (id.compare("enableEG") == 0) {
        if (key.compare("current") == 0) {
            enableEG = val;
            return;
        }
        if (key.compare("trig") == 0) {
            if (val >= -1 && val <= N_TRIGS)
                trig_enableEG = val;
            return;
        }
    }
    if (id.compare("attack") == 0) {
        if (key.compare("current") == 0) {
            attack = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_attack = val;
            return;
        }
    }
    if (id.compare("decay") == 0) {
        if (key.compare("current") == 0) {
            decay = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_decay = val;
            return;
        }
    }
    if (id.compare("sustain") == 0) {
        if (key.compare("current") == 0) {
            sustain = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_sustain = val;
            return;
        }
    }
    if (id.compare("release") == 0) {
        if (key.compare("current") == 0) {
            release = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_release = val;
            return;
        }
    }
    if (id.compare("root_frequency") == 0) {
        if (key.compare("current") == 0) {
            root_frequency = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_root_frequency = val;
            return;
        }
    }
    if (id.compare("root_bwidth") == 0) {
        if (key.compare("current") == 0) {
            root_bwidth = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_root_bwidth = val;
            return;
        }
    }
    if (id.compare("root_level") == 0) {
        if (key.compare("current") == 0) {
            root_level = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_root_level = val;
            return;
        }
    }
// sectionCpp0


    int index;
    index = id.rfind("_harm");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        //TBD_LOGE("SS", "harm index %d", index);
        if (key.compare("current") == 0) {
            p_harm[index] = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_p_harm[index] = val;
            return;
        }
    }
    index = id.rfind("_bwidth");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        //TBD_LOGE("SS", "bwidth index %d", index);
        if (key.compare("current") == 0) {
            p_bwidth[index] = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_p_bwidth[index] = val;
            return;
        }
    }
    index = id.rfind("_gain");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        //TBD_LOGE("SS", "gain index %d", index);
        if (key.compare("current") == 0) {
            p_gain[index] = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_p_gain[index] = val;
            return;
        }
    }
    index = id.rfind("_modbwsrc");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        if (key.compare("current") == 0) {
            p_modbwsrc[index] = val;
            return;
        }
    }
    index = id.rfind("_modbw");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        if (key.compare("current") == 0) {
            p_modbw[index] = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_p_modbw[index] = val;
            return;
        }
    }
    index = id.rfind("_modgainsrc");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        if (key.compare("current") == 0) {
            p_modgainsrc[index] = val;
            return;
        }
    }
    index = id.rfind("_modgain");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1)) - 1;
        if (key.compare("current") == 0) {
            p_modgain[index] = val;
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_p_modgain[index] = val;
            return;
        }
    }
    index = id.rfind("_enableEG");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1));
        if (key.compare("current") == 0) {
            eg_enableEG[index] = val;
            return;
        }
        if (key.compare("trig") == 0) {
            if (val >= -1 && val <= N_TRIGS)
                trig_eg_enableEG[index] = val;
            return;
        }
    }
    index = id.rfind("_loopEG");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1));
        if (key.compare("current") == 0) {
            eg_loopEG[index] = val;
            return;
        }
        if (key.compare("trig") == 0) {
            if (val >= -1 && val <= N_TRIGS)
                trig_eg_loopEG[index] = val;
            return;
        }
    }
    index = id.rfind("_mode");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1));
        if (key.compare("current") == 0) {
            eg_mode[index] = val;
            return;
        }
        if (key.compare("trig") == 0) {
            if (val >= -1 && val <= N_TRIGS)
                trig_eg_mode[index] = val;
            return;
        }
    }
    index = id.rfind("_attack");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1));
        if (key.compare("current") == 0) {
            eg_attack[index] = val;
            //TBD_LOGE("AT", "attack %d is %d", index, val);
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_eg_attack[index] = val;
            return;
        }
    }
    index = id.rfind("_decay");
    if (index != string::npos) {
        index = stoi(id.substr(index - 1, 1));
        if (key.compare("current") == 0) {
            eg_decay[index] = val;
            //TBD_LOGE("dec", "decay %d is %d %d", index, val, eg_decay[index].load());
            return;
        }
        if (key.compare("cv") == 0) {
            if (val >= -1 && val <= N_CVS)
                cv_eg_decay[index] = val;
            return;
        }
    }

}

void ctagSoundProcessorSubSynth::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
    srcsel = model->GetParamValue("srcsel", "current");
    cv_srcsel = model->GetParamValue("srcsel", "cv");
    cascade = model->GetParamValue("cascade", "current");
    cv_cascade = model->GetParamValue("cascade", "cv");
    partials = model->GetParamValue("partials", "current");
    cv_partials = model->GetParamValue("partials", "cv");
    gain = model->GetParamValue("gain", "current");
    cv_gain = model->GetParamValue("gain", "cv");
    enableEG = model->GetParamValue("enableEG", "current");
    trig_enableEG = model->GetParamValue("enableEG", "trig");
    attack = model->GetParamValue("attack", "current");
    cv_attack = model->GetParamValue("attack", "cv");
    decay = model->GetParamValue("decay", "current");
    cv_decay = model->GetParamValue("decay", "cv");
    sustain = model->GetParamValue("sustain", "current");
    cv_sustain = model->GetParamValue("sustain", "cv");
    release = model->GetParamValue("release", "current");
    cv_release = model->GetParamValue("release", "cv");
    root_frequency = model->GetParamValue("root_frequency", "current");
    cv_root_frequency = model->GetParamValue("root_frequency", "cv");
    root_bwidth = model->GetParamValue("root_bwidth", "current");
    cv_root_bwidth = model->GetParamValue("root_bwidth", "cv");
    root_level = model->GetParamValue("root_level", "current");
    cv_root_level = model->GetParamValue("root_level", "cv");
// sectionCpp1


    // params of partials
    for (int i = 0; i < 9; i++) {
        p_harm[i] = model->GetParamValue("p" + to_string(i + 1) + "_harm", "current");
        cv_p_harm[i] = model->GetParamValue("p" + to_string(i + 1) + "_harm", "cv");
        p_bwidth[i] = model->GetParamValue("p" + to_string(i + 1) + "_bwidth", "current");
        cv_p_bwidth[i] = model->GetParamValue("p" + to_string(i + 1) + "_bwidth", "cv");
        p_gain[i] = model->GetParamValue("p" + to_string(i + 1) + "_gain", "current");
        cv_p_gain[i] = model->GetParamValue("p" + to_string(i + 1) + "_gain", "cv");
        p_modbwsrc[i] = model->GetParamValue("p" + to_string(i + 1) + "_modbwsrc", "current");
        p_modbw[i] = model->GetParamValue("p" + to_string(i + 1) + "_modbw", "current");
        cv_p_modbw[i] = model->GetParamValue("p" + to_string(i + 1) + "_modbw", "cv");
        p_modgainsrc[i] = model->GetParamValue("p" + to_string(i + 1) + "_modgainsrc", "current");
        p_modgain[i] = model->GetParamValue("p" + to_string(i + 1) + "_modgain", "current");
        cv_p_modgain[i] = model->GetParamValue("p" + to_string(i + 1) + "_modgain", "cv");
    }

    // params of egs
    for (int i = 0; i < 2; i++) {
        eg_enableEG[i] = model->GetParamValue("eg" + to_string(i) + "_enableEG", "current");
        trig_eg_enableEG[i] = model->GetParamValue("eg" + to_string(i) + "_enableEG", "trig");
        eg_loopEG[i] = model->GetParamValue("eg" + to_string(i) + "_loopEG", "current");
        trig_eg_loopEG[i] = model->GetParamValue("eg" + to_string(i) + "_loopEG", "trig");
        eg_mode[i] = model->GetParamValue("eg" + to_string(i) + "_mode", "current");
        trig_eg_mode[i] = model->GetParamValue("eg" + to_string(i) + "_mode", "trig");
        eg_attack[i] = model->GetParamValue("eg" + to_string(i) + "_attack", "current");
        cv_eg_attack[i] = model->GetParamValue("eg" + to_string(i) + "_attack", "cv");
        eg_decay[i] = model->GetParamValue("eg" + to_string(i) + "_decay", "current");
        cv_eg_decay[i] = model->GetParamValue("eg" + to_string(i) + "_decay", "cv");
    }
}

#define LOG_2 0.693147181f

// original filter coeffs from zynaddsubfx, are pretty much the same as for esp dsp library
void ctagSoundProcessorSubSynth::computeFilterCoefs(float *coeffs,
                                                    float freq,
                                                    float bw,
                                                    float gain) {
    if (freq > fs / 2.0f - 200.0f)
        freq = fs / 2.0f - 200.0f;


    float omega = 2.0f * M_PI * freq / fs;
    float sn = HELPERS::fastsin(omega);
    float cs = HELPERS::fastcos(omega);
    float alpha = sn * HELPERS::fastsinh(LOG_2 / 2.0f * bw * omega / sn);

    if (alpha > 1)
        alpha = 1;
    if (alpha > bw)
        alpha = bw;

    coeffs[0] = alpha / (1.0f + alpha) * gain;
    coeffs[2] = -alpha / (1.0f + alpha) * gain;
    coeffs[3] = -2.0f * cs / (1.0f + alpha);
    coeffs[4] = (1.0f - alpha) / (1.0f + alpha);
}


float ctagSoundProcessorSubSynth::computeRolloff(float freq) {
    const float lower_limit = 10.0f;
    const float lower_width = 10.0f;
    const float upper_width = 200.0f;
    const float upper_limit = fs / 2.f;

    if (freq > lower_limit + lower_width &&
        freq < upper_limit - upper_width)
        return 1.0f;
    if (freq <= lower_limit || freq >= upper_limit)
        return 0.0f;
    if (freq <= lower_limit + lower_width)
        return (1.0f - HELPERS::fastcos(M_PI * (freq - lower_limit) / lower_width)) / 2.0f;
    return (1.0f - HELPERS::fastcos(M_PI * (freq - upper_limit) / upper_width)) / 2.0f;
}

void ctagSoundProcessorSubSynth::fetchControlData(const ProcessData &data) {
    if (cv_root_bwidth == -1)
        fRootBWidth = (float) root_bwidth / 4095.f;
    else
        fRootBWidth = data.cv[cv_root_bwidth] * data.cv[cv_root_bwidth];
    if (cv_root_frequency == -1)
        fRootFrequency = (float) root_frequency;
    else
        fRootFrequency = 0.9f * fRootFrequency +
                         0.1f * (float) root_frequency * HELPERS::fastpow2(data.cv[cv_root_frequency] * 5.f);
    if (cv_root_level == -1)
        fRootLevel = (float) root_level / 4095.f;
    else
        fRootLevel = data.cv[cv_root_level] * data.cv[cv_root_level];

    if (cv_gain == -1)
        fSumGain = (float) gain / 2048.f; //max 2 gain
    else
        fSumGain = (data.cv[cv_gain] * data.cv[cv_gain]) * 2.f;

    for (int i = 0; i < 9; i++) {
        if (cv_p_harm[i] == -1)
            fHarm[i] = HELPERS::fastpow2((float) p_harm[i] / 12.f);
        else
            fHarm[i] = HELPERS::fastpow2(data.cv[cv_p_harm[i]] * 9.f);

        if (cv_p_bwidth[i] == -1)
            fBWidth[i] = (float) p_bwidth[i] / 4095.f;
        else
            fBWidth[i] = data.cv[cv_p_bwidth[i]] * data.cv[cv_p_bwidth[i]];

        if (cv_p_gain[i] == -1)
            fGain[i] = (float) p_gain[i] / 2048.f; // max gain 2
        else
            fGain[i] = data.cv[cv_p_gain[i]] * data.cv[cv_p_gain[i]] * 2.f;

        if (cv_p_modbw[i] == -1)
            fModBW[i] = (float) p_modbw[i] / 4095.f;
        else
            fModBW[i] = data.cv[cv_p_modbw[i]];

        if (cv_p_modgain[i] == -1)
            fModGain[i] = (float) p_modgain[i] / 4095.f;
        else
            fModGain[i] = data.cv[cv_p_modgain[i]];
    }
}