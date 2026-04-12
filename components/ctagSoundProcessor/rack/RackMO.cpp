/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "RackSynth.hpp"
#include "RackMO.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void RackMO::Init(const PickSeqRackInitData *initdata) {
    mo_osc.Init();
    mo_osc.set_pitch(100);
    mo_last_shape = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW;
    mo_shape = static_cast<braids::MacroOscillatorShape>(mo_last_shape);
    mo_osc.set_shape(mo_last_shape);
    mo_ws.Init(0xcafe);
    //envelope.Init();
    mo_envelope.SetSampleRate(44100.f / 32.f);
    mo_envelope.SetModeExp();
    mo_quantizer.Init();

    
    initdata->rack->registerParamAndCC(initdata, "shape", 8, [&](const int val) { mo_shape = val; });
    initdata->rack->registerParamAndCC(initdata, "param_0", 9, [&](const int val) { mo_param_0 = val; });
    initdata->rack->registerParamAndCC(initdata, "param_1", 10, [&](const int val) { mo_param_1 = val; });
    initdata->rack->registerParamAndCC(initdata, "waveshaping", 11, [&](const int val) { mo_waveshaping = val; });
    
    initdata->rack->registerParamAndCC(initdata, "p0_amt", 12, [&](const int val) { mo_p0_amt = val; });
    initdata->rack->registerParamAndCC(initdata, "p1_amt", 13, [&](const int val) { mo_p1_amt = val; });
    initdata->rack->registerParamAndCC(initdata, "fm_amt", 14, [&](const int val) { mo_fm_amt = val; });
    initdata->rack->registerParamAndCC(initdata, "q_scale", 15, [&](const int val) { mo_q_scale = val; });
    
    initdata->rack->registerParamAndCC(initdata, "attack", 16, [&](const int val) { mo_attack = val; });
    initdata->rack->registerParamAndCC(initdata, "decay", 17, [&](const int val) { mo_decay = val; });
    initdata->rack->registerParamAndCC(initdata, "loopEG", 18, [&](const int val) { mo_loopEG = val; });
    
    initdata->rack->registerParamAndCC(initdata, "decimation", 19, [&](const int val) { mo_decimation = val; });
    initdata->rack->registerParamAndCC(initdata, "bit_reduction", 20, [&](const int val) { mo_bit_reduction = val; });
    // initdata->rack->registerParamAndCC(initdata, "pitch", 19, [&](const int val) { mo_pitch = val; });

    this->enabled = false;
}

void RackMO::noteOn(uint8_t note, uint8_t vel) {
    // TODO: Implement
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
    mo_pitch = note << 7; //  midi_freq * 128.0f; //   * 12.f * 5.f * 128.f  * 100.f; // 1/100 Hz per semitone
}

void RackMO::noteOff(uint8_t note, uint8_t vel) {
    mo_envelope.Reset();
}

void RackMO::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(mo_out, BUF_SZ, 0.f);

    // ad envelope and loop
    float a = mo_attack / 4095.f * 5.f;
    float d = mo_decay / 4095.f * 5.f;
    // if (cv_mo_attack != -1) {
    //     a = fabsf(data.cv[cv_mo_attack]) * 12.f;
    // }
    // if (cv_mo_decay != -1) {
    //     d = fabsf(data.cv[cv_mo_decay]) * 12.f;
    // }
    mo_envelope.SetAttack(a);
    mo_envelope.SetDecay(d);
    mo_envelope.SetLoop(mo_loopEG);
    int32_t ad_value = static_cast<uint32_t>(mo_envelope.Process() * 65535.f);

    // shape
    int s = mo_shape * 128 / 4096;
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;

    if (ms != mo_last_shape) {
        mo_last_shape = ms;
        // printf("MO shape=%d\n", ms);
        mo_osc.set_shape(ms);
    }

    bool trigger = false;
    if (midi_trig) {
        trigger = true;
        midi_trig = false;
    // } else if (trig_mo_enableEG != -1) {
    //     trigger = data.trig[trig_mo_enableEG] == 1 ? false : true;
    // } else {
    //     trigger = mo_enableEG;
    }

    if (!mo_prevTrigger && trigger) {
        // printf("MO\n");
        //envelope.Trigger(braids::EnvelopeSegment::ENV_SEGMENT_ATTACK);
        mo_envelope.Trigger();
        mo_osc.Strike();
    }
    mo_prevTrigger = trigger;

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = mo_param_0 * 32768 / 4096;
    parameters[1] = mo_param_1 * 32768 / 4096;
    // if (cv_mo_param_0 != -1) {
    //     parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_mo_param_0] * 32767));
    // }
    // if (cv_mo_param_1 != -1) {
    //     parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_mo_param_1] * 32767));
    // }
    int32_t mod_amt[2];
    mod_amt[0] = mo_p0_amt * 64 / 4096;
    mod_amt[1] = mo_p1_amt * 64 / 4096;
    int32_t mod[2];
    // if (cv_mo_p0_amt != -1) {
    //     mod[0] = static_cast<int32_t >(data.cv[cv_mo_p0_amt] * 65535.f);
    // } else {
        mod[0] = ad_value;
    // }
    // if (cv_mo_p1_amt != -1) {
        // mod[1] = static_cast<int32_t >(data.cv[cv_mo_p1_amt] * 65535.f);
    // } else {
        mod[1] = ad_value;
    // }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 128; // ad_value goes to 64k, 
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    mo_osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization + fm
    int32_t ipitch = mo_pitch;
    // if (cv_mo_pitch != -1) {
    //     ipitch += static_cast<int32_t>(data.cv[cv_mo_pitch] * 12.f * 5.f * 128.f); // five octaves
    // }
    int32_t sc = mo_q_scale * 47 / 4096;
    // if (cv_mo_q_scale != -1) {
    //     sc = static_cast<int32_t>(fabsf(data.cv[cv_mo_q_scale]) * 48.f);
    CONSTRAIN(sc, 0, 47);
    // }
    mo_quantizer.Configure(braids::scales[sc]);
    ipitch = mo_quantizer.Process(ipitch, mo_pitch);

    int32_t fm = mo_fm_amt * ad_value / 32768; // / 512;
    // if (cv_mo_fm_amt != -1) {
    //     fm = static_cast<int32_t>(data.cv[cv_mo_fm_amt] * 12.f * 3.f * 128.f); // three octaves
    // }
    ipitch += fm;
    CONSTRAIN(ipitch, 0, 16383);
    mo_osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[BUF_SZ];
    mo_osc.Render(mo_sync, buffer, BUF_SZ);

    // calculate amplitude modulation
    int32_t mod_gain = 65535; // a bit louder
    mod_gain = (ad_value) / 8;

    // convert final audio buffer
    int32_t sample = 0;
    uint16_t signature = mo_waveshaping * 16; // * 65535 / 4095;
    // if (cv_mo_waveshaping != -1) {
    //     signature = static_cast<uint16_t>(fabsf(data.cv[cv_mo_waveshaping]) * 65535.f);
    // }
    int32_t dfactor = (mo_decimation * 30 / 4096) + 1;
    // if (cv_mo_decimation != -1) {
    //     dfactor = static_cast<int32_t>(fabsf(data.cv[cv_mo_decimation]) * 30) + 1;
    // }
    int32_t br = mo_bit_reduction * 6 / 4096;
    // if (cv_mo_bit_reduction != -1) {
    //     br = static_cast<int32_t>(fabsf(data.cv[cv_mo_bit_reduction]) * 6);
    // }
    int16_t bit_mask = mo_bit_reduction_masks[6 - br];
    for (int i = 0; i < BUF_SZ; i++) {
        if ((i % dfactor) == 0) {
            sample = buffer[i] & bit_mask;
        }
        int16_t warped = mo_ws.Transform(sample);
        buffer[i] = stmlib::Mix(sample, warped, signature);
        buffer[i] = buffer[i] * mod_gain / 65535;
        mo_out[i] = static_cast<float>(buffer[i]) / 32767.f;
    }
}
