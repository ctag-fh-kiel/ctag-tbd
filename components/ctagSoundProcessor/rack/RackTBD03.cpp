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
#include "RackTBD03.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void RackTBD03::Init(const PickSeqRackInitData *initdata) {
    td3_pirkle_zdf_boost.Init();
    td3_karlson.Init();
    td3_blaukraut.Init();
    td3_pirkle_zdf.Init();
    td3_zavalishin.Init();
    td3_osc.Init();
    td3_osc.set_pitch(100);
    td3_osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    td3_adVCA.SetSampleRate(44100.f / 32);
    td3_adVCA.SetModeExp();
    td3_adVCA.SetAttack(0.f);
    td3_adVCA.SetDecay(0.5f);
    td3_adVCF.SetSampleRate(44100.f / 32);
    td3_adVCF.SetModeExp();
    td3_adVCF.SetAttack(0.f);
    td3_adVCF.SetDecay(0.5f);
    td3_ws.Init(0xcafe);

    initdata->rack->registerParamAndCC(initdata, "shape", 8, [&](const int val){ td3_shape = val;});
	initdata->rack->registerParamAndCC(initdata, "param_0", 9, [&](const int val){ td3_param_0 = val;});
    initdata->rack->registerParamAndCC(initdata, "decay_vca", 10, [&](const int val){ td3_decay_vca = val;});
    initdata->rack->registerParamAndCC(initdata, "decay_vcf", 11, [&](const int val){ td3_decay_vcf = val;});

    initdata->rack->registerParamAndCC(initdata, "cutoff", 12, [&](const int val){ td3_cutoff = val;});
	initdata->rack->registerParamAndCC(initdata, "resonance", 13, [&](const int val){ td3_resonance = val;});
	initdata->rack->registerParamAndCC(initdata, "envelope", 14, [&](const int val){ td3_envelope = val;});
	initdata->rack->registerParamAndCC(initdata, "filter_type", 15, [&](const int val){ td3_filter_type = val;});

	initdata->rack->registerParamAndCC(initdata, "saturation", 16, [&](const int val){ td3_saturation = val;});
    initdata->rack->registerParamAndCC(initdata, "drive", 17, [&](const int val){ td3_drive = val;});
    initdata->rack->registerParamAndCC(initdata, "slide", 18, [&](const int val){ td3_slide = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 19, [&](const int val){ td3_accent = val;});

    initdata->rack->registerParamAndCC(initdata, "param_1", 20, [&](const int val){ td3_param_1 = val;});
    initdata->rack->registerParamAndCC(initdata, "p0_amt", 21, [&](const int val){ td3_p0_amt = val;});
    initdata->rack->registerParamAndCC(initdata, "p1_amt", 22, [&](const int val){ td3_p1_amt = val;});
    initdata->rack->registerParamAndCC(initdata, "accent_level", 23, [&](const int val){ td3_accent_level = val;});

    initdata->rack->registerParamAndCC(initdata, "slide_level", 24, [&](const int val){ td3_slide_level = val;}); // not used
    initdata->rack->registerParamAndCC(initdata, "sync_trig", 25, [&](const int val){ td3_sync_trig = val;}); // not used

    this->enabled = false;
}

void RackTBD03::noteOn(uint8_t note, uint8_t vel) {
    midi_trig = true;
    midi_note = note;
    // midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
}

void RackTBD03::noteOff(uint8_t note, uint8_t vel) {
}

void RackTBD03::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(td3_out, BUF_SZ, 0.f);

    float dvcf, dvca;
    // bool trg = midi_trig;

    // if (trig_td3_trigger != -1) {
    //     trg = data.trig[trig_td3_trigger] == 1 ? 0 : 1; // negative logic
    // } else {
    //     trg = td3_trigger;
    // }

    if (midi_trig && !td3_pre_trig) {
        // printf("TBDD3\n");
        td3_isAccent = td3_accent;
        // if (trig_td3_accent != -1) {
        //     td3_isAccent = data.trig[trig_td3_accent] == 0 ? 1 : 0;
        // }
        dvcf = td3_decay_vcf / 4095.f * 5.f;
        // if (cv_td3_decay_vcf != -1) {
        //     dvcf = fabsf(data.cv[cv_td3_decay_vcf]) * 5.f;
        // }
        // if accent shorten decay of filter eg
        if (td3_isAccent) {
            dvcf = td3_kAccentDecay;
        }
        td3_adVCF.SetDecay(dvcf);
        dvca = td3_decay_vca / 4095.f * 5.f;
        // if (cv_td3_decay_vca != -1) {
        //     dvca = fabsf(data.cv[cv_td3_decay_vca]) * 5.f;
        // }
        td3_adVCA.SetDecay(dvca);
        td3_adVCF.Trigger();
        td3_adVCA.Trigger();
        // sync on trigger
        if (td3_sync_trig) td3_sync[0] = 1;
        td3_osc.Strike();
        td3_pre_trig = true;
        midi_trig = false;
    } else if (!midi_trig) {
        td3_pre_trig = false;
    }

    float egvalVCA = td3_adVCA.Process();
    // if accent make slightly louder
    if (td3_isAccent) {
        egvalVCA *= td3_kAccentVCAFactor;
    }
    float egvalVCF = td3_adVCF.Process();

    // shape — divisor matches RackMO.cpp:101 so all 47 Braids macro
    // shapes are reachable at full macro travel and each macro-source
    // increment of 1 advances exactly one shape (no jumping by 2-3 as
    // the previous `* 47 / 4096` divisor produced — that one only
    // reached 17 of 47 shapes and stepped fractionally per click).
    int s = td3_shape * 128 / 4096;
    // if (cv_td3_shape != -1) {
    //     s = fabsf(data.cv[cv_td3_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    // }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    td3_osc.set_shape(ms);

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = td3_param_0 * 32768 / 4096;
    parameters[1] = td3_param_1 * 32768 / 4096;
    // if (cv_td3_param_0 != -1) {
    //     parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_0] * 32767));
    // }
    // if (cv_td3_param_1 != -1) {
    //     parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_1] * 32767));
    // }
    int32_t mod_amt[2];
    mod_amt[0] = td3_p0_amt;// * 4096 / 4096;
    mod_amt[1] = td3_p1_amt;// * 4096 / 4096;
    int32_t mod[2];
    // if (cv_td3_p0_amt != -1) {
    //     mod[0] = static_cast<int32_t >(data.cv[cv_td3_p0_amt] * 65535.f);
    // } else {
        mod[0] = static_cast<int32_t >(egvalVCF * 65535.f);
    // }
    // if (cv_td3_p1_amt != -1) {
    //     mod[1] = static_cast<int32_t >(data.cv[cv_td3_p1_amt] * 65535.f);
    // } else {
        mod[1] = static_cast<int32_t >(egvalVCF * 65535.f);
    // }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 8192;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    td3_osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization
    MK_BOOL_PAR_NOCV(isSlide, td3_slide)
    MK_FLT_PAR_ABS_NOCV(fSlideLevel, td3_slide_level, 4095.f, 0.099f)
    fSlideLevel += 0.9f;
    int32_t ipitch = 0; // midi_freq * 128.0f;
    // if (cv_td3_pitch != -1) {
    float fPitch = midi_note; //  data.cv[cv_td3_pitch] * 12.f * 5.f; // five octaves
    // // if(isSlide){
    // //     fPitch = fSlideLevel * td3_pre_pitch_val + (1.f - fSlideLevel) * fPitch;
    // // }
    // td3_pre_pitch_val = fPitch;
    ipitch += static_cast<int32_t>(fPitch * 128.f);
    // }
    CONSTRAIN(ipitch, 0, 16383);
    td3_osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[BUF_SZ];
    td3_osc.Render(td3_sync, buffer, BUF_SZ);

    // apply filter and EGs
    // 5 filter types (0..4 — Pirkle ZDF+boost / Karlson / Blaukraut /
    // Pirkle ZDF / Zavalishin). The previous `* 4 / 4096` divisor only
    // reached 3 at full atomic travel, leaving filter 4 (Zavalishin)
    // unreachable. The macro now exposes max:4 mul:31 (small-enum
    // pattern from UX PRINCIPLE #4: floor(127/(N-1)) = 31 for N=5),
    // so atomic spans 0..3968 across the 5 source steps. `* 5 / 4096`
    // produces 0..4.84, rounded to 0..4 cleanly.
    int ftype = (td3_filter_type * 5) / 4096;
    CONSTRAIN(ftype, 0, 4)

    // Cutoff log-mapped 20 Hz..22 kHz — the previous linear formula
    // `c = 27000 * x - 5000` (clamped to [20, 22000]) wasted the bottom
    // 18.6% of the knob (silently below 20 Hz) and compressed 20-150 Hz
    // into 0.5% of travel. Reporter heard "stepping artifacts" in that
    // dead zone, which were the linear quantisation showing through —
    // log mapping collapses those to ~3 cents per atomic step.
    // 1100 ≈ 22000/20, so cnorm=0 → 20 Hz, cnorm=1 → 22 kHz; equal
    // octaves per fraction of knob travel.
    float cnorm = td3_cutoff / 4095.f;
    CONSTRAIN(cnorm, 0.f, 1.f)
    float c = 20.f * powf(1100.f, cnorm);
    float fenv = td3_envelope / 4095.f;
    // if (cv_td3_envelope != -1) {
    //     fenv = fabsf(data.cv[cv_td3_envelope]);
    // }
    c += fenv * egvalVCF * 22000.f;
    // if accent add to VCF envelope
    float facclev = td3_accent_level / 4095.f;
    // if (cv_td3_accent_level != -1) {
    //     facclev = fabsf(data.cv[cv_td3_accent_level]);
    // }
    if (td3_isAccent) {
        c += facclev * egvalVCF * 22000.f;
    }

    float r = td3_resonance / 4095.f;
    // if (cv_td3_resonance != -1) {
    //     r = fabsf(data.cv[cv_td3_resonance]);
    // }

    int32_t signature = td3_saturation * 16; // * 65535 / 4095;
    // if (cv_td3_saturation != -1) {
    //     signature = static_cast<int32_t>(fabsf(data.cv[cv_td3_saturation]) * 65535.f);
    // }
    CONSTRAIN(signature, 0, 65535)

    float dri = td3_drive / 4095.f * 30.f;
    // if (cv_td3_drive != -1) {
    //     dri = fabsf(data.cv[cv_td3_drive]) * 30.f;
    // }

    CONSTRAIN(c, 20.f, 22000.f)
    CONSTRAIN(r, 0.f, 1.f)
    CONSTRAIN(dri, 1.f, 30.f)
    ctagFilterBase *filter = &td3_pirkle_zdf_boost;
    switch(ftype){
        case 0:
            filter = &td3_pirkle_zdf_boost;
            break;
        case 1:
            filter = &td3_karlson;
            break;
        case 2:
            filter = &td3_blaukraut;
            break;
        case 3:
            filter = &td3_pirkle_zdf;
            break;
        case 4:
            filter = &td3_zavalishin;
            break;
    }
    filter->SetCutoff(c);
    filter->SetResonance(r);
    filter->SetGain(dri);

    for (int i = 0; i < BUF_SZ; i++) {
        float eg = td3_pre_eg_val +
                   (egvalVCA - td3_pre_eg_val) / (float) BUF_SZ * i; // linear fade from previous eg value to avoid glitches
        // apply non linearity to filter input
        int16_t warped = td3_ws.Transform(buffer[i]);
        buffer[i] = stmlib::Mix(buffer[i], warped, signature);
        // filter, EG and clip
        const float div = 3.0518509476E-5f;

        float f = stmlib::SoftClip(eg * filter->Process(buffer[i] * div));
        td3_out[i] = f;
    }
    td3_pre_eg_val = egvalVCA;
    // sync on trigger
    td3_sync[0] = 0;
}
