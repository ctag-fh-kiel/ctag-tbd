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

#include <tbd/sounds/SoundProcessorTBD03.hpp>
#include <iostream>
#include <cmath>
#include <tbd/sound_utils/ctagFastMath.hpp>
#include "stmlib/dsp/dsp.h"

// Details on mode of action of envelope generators
// https://www.firstpr.com.au/rwi/dfish/303-unique.html

using namespace tbd::sounds;
using namespace stmlib;

#define kAccentDecay 0.5f
#define kAccentVCAFactor 1.5f

void SoundProcessorTBD03::Init(std::size_t blockSize, void *blockPtr) {
    pirkle_zdf_boost.Init();
    karlson.Init();
    blaukraut.Init();
    pirkle_zdf.Init();
    zavalishin.Init();

    osc.Init();
    osc.set_pitch(100);
    osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    adVCA.SetSampleRate(44100.f / bufSz);
    adVCA.SetModeExp();
    adVCA.SetAttack(0.f);
    adVCA.SetDecay(0.5f);

    adVCF.SetSampleRate(44100.f / bufSz);
    adVCF.SetModeExp();
    adVCF.SetAttack(0.f);
    adVCF.SetDecay(0.5f);

    ws.Init(0xcafe);
}   

void SoundProcessorTBD03::Process(const sound_processor::ProcessData&data) {
    float dvcf, dvca;
    bool trg;

    if (trig_trigger != -1) {
        trg = data.trig[trig_trigger] == 1 ? 0 : 1; // negative logic
    } else {
        trg = trigger;
    }

    if (trg && !pre_trig) {
        isAccent = accent;
        if (trig_accent != -1) {
            isAccent = data.trig[trig_accent] == 0 ? 1 : 0;
        }
        dvcf = decay_vcf / 4095.f * 5.f;
        if (cv_decay_vcf != -1) {
            dvcf = fabsf(data.cv[cv_decay_vcf]) * 5.f;
        }
        // if accent shorten decay of filter eg
        if (isAccent) {
            dvcf = kAccentDecay;
        }
        adVCF.SetDecay(dvcf);
        dvca = decay_vca / 4095.f * 5.f;
        if (cv_decay_vca != -1) {
            dvca = fabsf(data.cv[cv_decay_vca]) * 5.f;
        }
        adVCA.SetDecay(dvca);
        adVCF.Trigger();
        adVCA.Trigger();
        // sync on trigger
        if (sync_trig) sync[0] = 1;
        osc.Strike();
        pre_trig = true;
    } else if (!trg) {
        pre_trig = false;
    }

    float egvalVCA = adVCA.Process();
    // if accent make slightly louder
    if (isAccent) {
        egvalVCA *= kAccentVCAFactor;
    }
    float egvalVCF = adVCF.Process();

    // shape
    int s = shape;
    if (cv_shape != -1) {
        s = fabsf(data.cv[cv_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    osc.set_shape(ms);

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
        mod[0] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    if (cv_p1_amt != -1) {
        mod[1] = static_cast<int32_t >(data.cv[cv_p1_amt] * 65535.f);
    } else {
        mod[1] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 8192;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization
    MK_BOOL_PAR(isSlide, slide)
    MK_FLT_PAR_ABS(fSlideLevel, slide_level, 4095.f, 0.099f)
    fSlideLevel += 0.9f;
    int32_t ipitch = pitch;
    if (cv_pitch != -1) {
        float fPitch = data.cv[cv_pitch] * 12.f * 5.f; // five octaves
        if(isSlide){
            fPitch = fSlideLevel * pre_pitch_val + (1.f - fSlideLevel) * fPitch;
        }
        pre_pitch_val = fPitch;
        ipitch += static_cast<int32_t>(fPitch * 128.f);
    }
    CONSTRAIN(ipitch, 0, 16383);
    osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[32];
    osc.Render(sync, buffer, bufSz);

    // apply filter and EGs
    int ftype = filter_type;
    if (cv_filter_type != -1) {
        ftype = static_cast<int>(fabsf(data.cv[cv_filter_type]) * 5.f);
    }
    CONSTRAIN(ftype, 0, 4)
    float c = cutoff / 4095.f;
    if (cv_cutoff != -1) {
        c = fabsf(data.cv[cv_cutoff]);
    }
    c *= 27000.f;
    c -= 5000.f;
    float fenv = envelope / 4095.f;
    if (cv_envelope != -1) {
        fenv = fabsf(data.cv[cv_envelope]);
    }
    c += fenv * egvalVCF * 22000.f;
    // if accent add to VCF envelope
    float facclev = accent_level / 4095.f;
    if (cv_accent_level != -1) {
        facclev = fabsf(data.cv[cv_accent_level]);
    }
    if (isAccent) {
        c += facclev * egvalVCF * 22000.f;
    }

    float r = resonance / 4095.f;
    if (cv_resonance != -1) {
        r = fabsf(data.cv[cv_resonance]);
    }

    int32_t signature = saturation;
    if (cv_saturation != -1) {
        signature = static_cast<int32_t>(fabsf(data.cv[cv_saturation]) * 65535.f);
    }
    CONSTRAIN(signature, 0, 65535)

    float dri = drive / 4095.f * 30.f;
    if (cv_drive != -1) {
        dri = fabsf(data.cv[cv_drive]) * 30.f;
    }

    CONSTRAIN(c, 20.f, 22000.f)
    CONSTRAIN(r, 0.f, 1.f)
    CONSTRAIN(dri, 1.f, 30.f)
    ctagFilterBase *filter = &pirkle_zdf_boost;
    switch(ftype){
        case 0:
            filter = &pirkle_zdf_boost;
            break;
        case 1:
            filter = &karlson;
            break;
        case 2:
            filter = &blaukraut;
            break;
        case 3:
            filter = &pirkle_zdf;
            break;
        case 4:
            filter = &zavalishin;
            break;
    }
    filter->SetCutoff(c);
    filter->SetResonance(r);
    filter->SetGain(dri);

    float fgain = gain / 4095.f * 2.f;
    if (cv_gain != -1) {
        fgain = fabsf(data.cv[cv_gain]) * 2.f;
    }

    for (int i = 0; i < bufSz; i++) {
        float eg = pre_eg_val +
                   (egvalVCA - pre_eg_val) / (float) bufSz * i; // linear fade from previous eg value to avoid glitches
        // apply non linearity to filter input
        int16_t warped = ws.Transform(buffer[i]);
        buffer[i] = stmlib::Mix(buffer[i], warped, signature);
        // filter, EG and clip
        const float div = 3.0518509476E-5f;
        data.buf[i * 2 + processCh] = fgain * stmlib::SoftClip(eg * filter->Process(buffer[i] * div));
    }
    pre_eg_val = egvalVCA;
    // sync on trigger
    sync[0] = 0;
}
