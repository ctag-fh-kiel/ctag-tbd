/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2023 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <tbd/sounds/SoundProcessorMacOscDuo.hpp>
#include <cmath>
#include "braids/quantizer_scales.h"

using namespace tbd::sounds;

void SoundProcessorMacOscDuo::Process(const sound_processor::ProcessData&data) {
    // ad envelope and loop
    float a = attack / 4095.f * 5.f;
    float d = decay / 4095.f * 5.f;
    float s = sustain / 4095.f;
    float r = release / 4095.f * 5.f;
    if (cv_attack != -1) {
        a = fabsf(data.cv[cv_attack]) * 12.f;
    }
    if (cv_decay != -1) {
        d = fabsf(data.cv[cv_decay]) * 12.f;
    }
    if (cv_sustain != -1) {
        s = fabsf(data.cv[cv_sustain]);
    }
    if (cv_release != -1) {
        r = fabsf(data.cv[cv_release]) * 12.f;
    }

    int32_t ad_value[2];
    for(int i {0};i<2;i++){
        envelope[i].SetAttack(a);
        envelope[i].SetDecay(d);
        envelope[i].SetSustain(s);
        envelope[i].SetRelease(r);
        envelopeHighRes[i].SetAttack(a);
        envelopeHighRes[i].SetDecay(d);
        envelopeHighRes[i].SetSustain(s);
        envelopeHighRes[i].SetRelease(r);
        ad_value[i] = static_cast<uint32_t>(envelope[i].Process() * 65535.f);
    }

    // lfo for both
    float lfo_freq = lfo_f / 4095.f * 30.f;
    if(cv_lfo_f != -1){
        lfo_freq = fabsf(data.cv[cv_lfo_f]) * 30.f;
    }
    lfo.SetFrequency(lfo_freq);
    lfoHighRes.SetFrequency(lfo_freq);
    int32_t lfo_val = static_cast<uint32_t>(lfo.Process() * 65535.f / 2.f);

    // shape
    int shp = shape;
    if (cv_shape != -1) {
        shp = fabsf(data.cv[cv_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(shp);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    osc[0].set_shape(ms);
    osc[1].set_shape(ms);

    // trigger
    int gate[2];
    gate[0] = enableEG1;
    gate[1] = enableEG2;
    int trig_gate[2];
    trig_gate[0] = trig_enableEG1;
    trig_gate[1] = trig_enableEG2;
    for(int i {0};i<2;i++){ // strike oscillators
        if (gate[i] == 1 && trig_gate[i] == -1) {
            if (prevTrigger[i] == false) {
                osc[i].Strike();
            }
            prevTrigger[i] = true;
        } else if (gate[i] == 1 && trig_gate[i] != -1) {
            bool trigger = data.trig[trig_gate[i]] == 1 ? false : true;
            if (prevTrigger[i] == false && trigger) {
                osc[i].Strike();
            }
            prevTrigger[i] = trigger;
        } else {
            prevTrigger[i] = false;
        }
        if(trig_gate[i] != -1){
            gate[i] = data.trig[trig_gate[i]] == 1 ? false : true;
        }
        envelope[i].Gate(gate[i]);
        envelopeHighRes[i].Gate(gate[i]);
    }

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2][2]; // param, voice #
    parameters[0][0] = param_0;
    parameters[0][1] = param_0;
    parameters[1][0] = param_1;
    parameters[1][1] = param_1;
    int32_t mod_amt[2];
    mod_amt[0] = p0_amt;
    mod_amt[1] = p1_amt;
    int32_t mod[2][2]; // mod, voice #
    for(int i {0};i<2;i++){
        if (cv_param_0 != -1) {
            parameters[0][i] = static_cast<int16_t>(fabsf(data.cv[cv_param_0] * 32767));
        }
        if (cv_param_1 != -1) {
            parameters[1][i] = static_cast<int16_t>(fabsf(data.cv[cv_param_1] * 32767));
        }
        if (cv_p0_amt != -1) {
            float v = 0.9f * smoothp0[i] + 0.1f * data.cv[cv_p0_amt] * static_cast<float>(ad_value[i]);
            smoothp0[i] = v;
            mod[0][i] = static_cast<int32_t >(v);
        } else {
            mod[0][i] = ad_value[i];
        }
        if (cv_p1_amt != -1) {
            float v = 0.9f * smoothp1[i] + 0.1f * data.cv[cv_p1_amt] * static_cast<float>(ad_value[i]);
            smoothp1[i] = v;
            mod[1][i] = static_cast<int32_t >(v);
        } else {
            mod[1][i] = ad_value[i];
        }
    }
    for(int j {0};j<2;j++){ // voice
        for (int i = 0; i < 2; ++i) { // mod
            int32_t value = parameters[i][j];
            value += (mod[i][j] * mod_amt[i]) / 64;
            CONSTRAIN(value, 0, 32767);
            parameters[i][j] = static_cast<int16_t>(value);
        }
        osc[j].set_parameters(parameters[0][j], parameters[1][j]);
    }

    // pitch calculation and quantization + fm
    // lfo
    MK_INT_PAR(iFMAmt2, fm_amt2, 1024)

    // tune
    float fTune1 = tune1 / 4095.f * 12.f * 128.f;
    if(cv_tune1 != -1)
        fTune1 = data.cv[cv_tune1] * 12.f * 128.f;
    float fTune2 = tune2 / 4095.f * 12.f * 128.f;
    if(cv_tune2 != -1)
        fTune2 = data.cv[cv_tune2] * 12.f * 128.f;
    for(int i {0};i<2;i++){
        int32_t ipitch = i == 0 ? pitch1 : pitch2;
        if (i == 0 && cv_pitch1 != -1) {
            ipitch += static_cast<int32_t>(data.cv[cv_pitch1] * 12.f * 5.f * 128.f); // five octaves
        }
        if (i == 1 && cv_pitch2 != -1) {
            ipitch += static_cast<int32_t>(data.cv[cv_pitch2] * 12.f * 5.f * 128.f); // five octaves
        }
        int32_t sc = q_scale;
        if (cv_q_scale != -1) {
            sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
            CONSTRAIN(sc, 0, 47);
        }
        quantizer[i].Configure(braids::scales[sc]);
        ipitch = quantizer[i].Process(ipitch, i == 0 ? pitch1 : pitch2);

        // fm
        int fm = iFMAmt2 * lfo_val / 8192; // lfo
        int iFMAmt1 = fm_amt1;
        if(cv_fm_amt1 != -1) iFMAmt1 = static_cast<int>(data.cv[cv_fm_amt1] * 127.f) - 64; // bipolar
        fm += iFMAmt1 * ad_value[i] / 1024; // adsr

        ipitch += fm;
        ipitch += (i == 0 ? static_cast<int32_t>(fTune1) : static_cast<int32_t>(fTune2));
        CONSTRAIN(ipitch, 0, 16383);
        osc[i].set_pitch(static_cast<int16_t >(ipitch));
    }

    // render audio data
    int16_t buffer1[32];
    int16_t buffer2[32];
    osc[0].Render(sync1, buffer1, bufSz);
    osc[1].Render(sync2, buffer2, bufSz);

    //  amplitude modulation
    MK_FLT_PAR_ABS(fAM, am_amt, 64.f, 1.f)

    // convert final audio buffer
    uint16_t signature = waveshaping;
    if (cv_waveshaping != -1) {
        signature = static_cast<uint16_t>(fabsf(data.cv[cv_waveshaping]) * 65535.f);
    }
    int32_t dfactor = decimation;
    if (cv_decimation != -1) {
        dfactor = static_cast<int32_t>(fabsf(data.cv[cv_decimation]) * 30) + 1;
    }
    int32_t br = bit_reduction;
    if (cv_bit_reduction != -1) {
        br = static_cast<int32_t>(fabsf(data.cv[cv_bit_reduction]) * 6);
    }
    int16_t bit_mask = bit_reduction_masks[6 - br];
    float fGain1 = static_cast<float>(gain1) / 4095.f * 1.5f;
    float fGain2 = static_cast<float>(gain2) / 4095.f * 1.5f;
    if(cv_gain1 != -1){
        fGain1 *= fabsf(data.cv[cv_gain1]);
    }
    if(cv_gain2 != -1){
        fGain2 *= fabsf(data.cv[cv_gain2]);
    }
    int16_t sample1 = 0;
    int16_t sample2 = 0;
    for (int i = 0; i < bufSz; i++) {
        if ((i % dfactor) == 0) {
            sample1 = buffer1[i] & bit_mask;
            sample2 = buffer2[i] & bit_mask;
        }
        int16_t warped1 = ws[0].Transform(sample1);
        int16_t warped2 = ws[1].Transform(sample2);
        buffer1[i] = stmlib::Mix(sample1, warped1, signature);
        buffer2[i] = stmlib::Mix(sample2, warped2, signature);
        float s1 = static_cast<float>(buffer1[i]) / 32767.f * fGain1;
        float s2 = static_cast<float>(buffer2[i]) / 32767.f * fGain2;
        float fLFOhr = lfoHighRes.Process();
        float amFactor1 = envelopeHighRes[0].Process() * (1.f + fAM * fLFOhr);
        float amFactor2 = envelopeHighRes[1].Process() * (1.f + fAM * fLFOhr);
        s1 *= amFactor1;
        s2 *= amFactor2;
        data.buf[i * 2 + this->processCh] = s1 + s2;
    }
}

void SoundProcessorMacOscDuo::Init(std::size_t blockSize, void *blockPtr) {
    for(int i {0};i<2;i++){
        osc[i].Init();
        osc[i].set_pitch(100);
        osc[i].set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
        ws[i].Init(0xcafe);
        //envelope[i].Init();
        envelope[i].SetSampleRate(44100.f / 32.f);
        envelope[i].SetModeExp();
        envelope[i].Reset();
        envelopeHighRes[i].SetSampleRate(44100.f);
        envelopeHighRes[i].SetModeExp();
        envelopeHighRes[i].Reset();
        quantizer[i].Init();
        lfo.SetSampleRate(44100.f / 32.f);
        lfo.SetFrequency(1.f);
        lfoHighRes.SetSampleRate(44100.f);
        lfoHighRes.SetFrequency(1.f);
    }
}

SoundProcessorMacOscDuo::~SoundProcessorMacOscDuo() {
}
