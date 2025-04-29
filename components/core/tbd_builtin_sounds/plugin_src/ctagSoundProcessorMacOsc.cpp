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

#include <tbd/sounds/SoundProcessorMacOsc.hpp>
#include <iostream>
#include <cmath>
#include "braids/quantizer_scales.h"

using namespace tbd::sounds;

void SoundProcessorMacOsc::Init(std::size_t blockSize, void *blockPtr) {
    osc.Init();
    osc.set_pitch(100);
    osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    ws.Init(0xcafe);
    //envelope.Init();
    envelope.SetSampleRate(44100.f / 32.f);
    envelope.SetModeExp();
    quantizer.Init();
}

void SoundProcessorMacOsc::Process(const audio::ProcessData&data) {
    // ad envelope and loop
    float a = attack / 4095.f * 5.f;
    float d = decay / 4095.f * 5.f;
    if (cv_attack != -1) {
        a = fabsf(data.cv[cv_attack]) * 12.f;
    }
    if (cv_decay != -1) {
        d = fabsf(data.cv[cv_decay]) * 12.f;
    }
    envelope.SetAttack(a);
    envelope.SetDecay(d);
    if (trig_loopEG != -1) {
        envelope.SetLoop(data.trig[trig_loopEG] == 1 ? false : true);
    } else {
        envelope.SetLoop(loopEG);
    }
    int32_t ad_value = static_cast<uint32_t>(envelope.Process() * 65535.f);

    // shape
    int s = shape;
    if (cv_shape != -1) {
        s = fabsf(data.cv[cv_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    osc.set_shape(ms);

    // trigger
    if (enableEG == 1 && trig_enableEG == -1) {
        if (prevTrigger == false) {
            //envelope.Trigger(braids::EnvelopeSegment::ENV_SEGMENT_ATTACK);
            envelope.Trigger();
            osc.Strike();
        }
        prevTrigger = true;
    } else if (enableEG == 1 && trig_enableEG != -1) {
        bool trigger = data.trig[trig_enableEG] == 1 ? false : true;
        if (prevTrigger == false && trigger) {
            //envelope.Trigger(braids::EnvelopeSegment::ENV_SEGMENT_ATTACK);
            envelope.Trigger();
            osc.Strike();
        }
        prevTrigger = trigger;
    } else {
        prevTrigger = false;
    }

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = param_0;
    parameters[1] = param_1;
    if (cv_param_0 != -1) {
        parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_param_0] * 32767));
    }
    if (cv_param_1 != -1) {
        parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_param_1] * 32767));
    }
    int32_t mod_amt[2];
    mod_amt[0] = p0_amt;
    mod_amt[1] = p1_amt;
    int32_t mod[2];
    if (cv_p0_amt != -1) {
        mod[0] = static_cast<int32_t >(data.cv[cv_p0_amt] * 65535.f);
    } else {
        mod[0] = ad_value;
    }
    if (cv_p1_amt != -1) {
        mod[1] = static_cast<int32_t >(data.cv[cv_p1_amt] * 65535.f);
    } else {
        mod[1] = ad_value;
    }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 64;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization + fm
    int32_t ipitch = pitch;
    if (cv_pitch != -1) {
        ipitch += static_cast<int32_t>(data.cv[cv_pitch] * 12.f * 5.f * 128.f); // five octaves
    }
    int32_t sc = q_scale;
    if (cv_q_scale != -1) {
        sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
        CONSTRAIN(sc, 0, 47);
    }
    quantizer.Configure(braids::scales[sc]);
    ipitch = quantizer.Process(ipitch, pitch);

    int32_t fm = fm_amt * ad_value / 512;
    if (cv_fm_amt != -1) {
        fm = static_cast<int32_t>(data.cv[cv_fm_amt] * 12.f * 3.f * 128.f); // three octaves
    }
    ipitch += fm;
    CONSTRAIN(ipitch, 0, 16383);
    osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[32];
    osc.Render(sync, buffer, bufSz);

    // calculate amplitude modulation
    int32_t am = am_amt;
    int32_t mod_gain = 65535;
    if (am > 0) mod_gain -= am * (65535 - ad_value) / 64;
    if (am < 0) mod_gain += am * ad_value / 64;
    if (cv_am_amt != -1) {
        mod_gain = static_cast<int32_t>(data.cv[cv_am_amt] * 65535.f);
    }

    // convert final audio buffer
    int32_t sample = 0;
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
    float fGain = gain / 4095.f * 1.5f;
    for (int i = 0; i < 32; i++) {
        if ((i % dfactor) == 0) {
            sample = buffer[i] & bit_mask;
        }
        int16_t warped = ws.Transform(sample);
        buffer[i] = stmlib::Mix(sample, warped, signature);
        buffer[i] = buffer[i] * mod_gain / 65535;
        data.buf[i * 2 + this->processCh] = static_cast<float>(buffer[i]) / 32767.f * fGain;
    }
}
