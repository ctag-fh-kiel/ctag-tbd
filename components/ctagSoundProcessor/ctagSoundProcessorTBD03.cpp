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

#include "ctagSoundProcessorTBD03.hpp"
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"
#include "stmlib/dsp/dsp.h"

// Details on mode of action of envelope generators
// https://www.firstpr.com.au/rwi/dfish/303-unique.html

using namespace CTAG::SP;
using namespace stmlib;

#define kAccentDecay 0.5f
#define kAccentVCAFactor 1.5f

ctagSoundProcessorTBD03::ctagSoundProcessorTBD03() {
    setIsStereo();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);

    filt[0] = std::make_unique<ctagDiodeLadderFilter5>(); // Pirkle ZDF with boost
    filt[1] = std::make_unique<ctagDiodeLadderFilter3>(); // Karlson
    filt[2] = std::make_unique<ctagDiodeLadderFilter4>(); // Blaukraut
    filt[3] = std::make_unique<ctagDiodeLadderFilter>(); // Pirkle ZDF
    filt[4] = std::make_unique<ctagDiodeLadderFilter>(); // Zavalishin ZDF

    filt[0]->Init();
    filt[1]->Init();
    filt[2]->Init();
    filt[3]->Init();
    filt[4]->Init();

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

void ctagSoundProcessorTBD03::Process(const ProcessData &data) {
    float dvcf, dvca;
    bool trg;

    if(trig_trigger != -1) {
        trg = data.trig[trig_trigger] == 1 ? 0 : 1; // negative logic
    }else{
        trg = trigger;
    }

    if(trg && !pre_trig){
        isAccent = accent;
        if(trig_accent != -1){
            isAccent = data.trig[trig_accent] == 0 ? 1 : 0;
        }
        dvcf = decay_vcf / 4095.f * 5.f;
        if(cv_decay_vcf != -1){
            dvcf = fabsf(data.cv[cv_decay_vcf]) * 5.f;
        }
        // if accent shorten decay of filter eg
        if(isAccent){
            dvcf = kAccentDecay;
        }
        adVCF.SetDecay(dvcf);
        dvca = decay_vca / 4095.f * 5.f;
        if(cv_decay_vca != -1){
            dvca = fabsf(data.cv[cv_decay_vca]) * 5.f;
        }
        adVCA.SetDecay(dvca);
        adVCF.Trigger();
        adVCA.Trigger();
        // sync on trigger
        if(sync_trig) sync[0] = 1;
        osc.Strike();
        pre_trig = true;
    }else if(!trg){
        pre_trig = false;
    }

    float egvalVCA = adVCA.Process();
    // if accent make slightly louder
    if(isAccent){
        egvalVCA *= kAccentVCAFactor;
    }
    float egvalVCF = adVCF.Process();

    // shape
    int s = shape;
    if(cv_shape != -1){
        s = fabsf(data.cv[cv_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if(ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META) ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    osc.set_shape(ms);

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = param_0;
    parameters[1] = param_1;
    if(cv_param_0 != -1){
        parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_param_0] * 32767));
    }
    if(cv_param_1 != -1){
        parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_param_1] * 32767));
    }
    int32_t mod_amt[2];
    mod_amt[0] = p0_amt;
    mod_amt[1] = p1_amt;
    int32_t mod[2];
    if(cv_p0_amt != -1){
        mod[0] = static_cast<int32_t >(data.cv[cv_p0_amt] * 65535.f);
    }else{
        mod[0] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    if(cv_p1_amt != -1){
        mod[1] = static_cast<int32_t >(data.cv[cv_p1_amt] * 65535.f);
    }else{
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
    int32_t ipitch = pitch;
    if(cv_pitch != -1){
        ipitch += static_cast<int32_t>(fabsf(data.cv[cv_pitch] * 12.f * 5.f * 128.f)); // five octaves
    }
    CONSTRAIN(ipitch, 0, 16383);
    osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[32];
    osc.Render(sync, buffer, bufSz);

    // apply filter and EGs
    int ftype = filter_type;
    if(cv_filter_type != -1){
        ftype = static_cast<int>(fabsf(data.cv[cv_filter_type]) * 5.f);
    }
    CONSTRAIN(ftype, 0, 4)
    float c = cutoff / 4095.f;
    if(cv_cutoff != -1){
        c = fabsf(data.cv[cv_cutoff]);
    }
    c *= 27000.f;
    c -= 5000.f;
    float fenv = envelope / 4095.f;
    if(cv_envelope != -1){
        fenv = fabsf(data.cv[cv_envelope]);
    }
    c += fenv * egvalVCF * 22000.f;
    // if accent add to VCF envelope
    float facclev = accent_level / 4095.f;
    if(cv_accent_level != -1){
        facclev = fabsf(data.cv[cv_accent_level]);
    }
    if(isAccent){
        c += facclev * egvalVCF * 22000.f;
    }
    CONSTRAIN(c, 20.f, 22000.f)
    filt[ftype]->SetCutoff(c);

    float r = resonance / 4095.f;
    if(cv_resonance != -1){
        r = fabsf(data.cv[cv_resonance]);
    }
    filt[ftype]->SetResonance(r);

    int32_t signature = saturation;
    if(cv_saturation != -1){
        signature = static_cast<int32_t>(fabsf(data.cv[cv_saturation]) * 65535.f);
    }
    CONSTRAIN(signature, 0, 65535)

    float dri = drive / 4095.f * 30.f;
    if(cv_drive != -1){
        dri = fabsf(data.cv[cv_drive]) * 30.f;
    }
    CONSTRAIN(dri, 1.f, 30.f)
    filt[ftype]->SetGain(dri);

    float fgain = gain / 4095.f * 2.f;
    if(cv_gain != -1){
        fgain = fabsf(data.cv[cv_gain]) * 2.f;
    }

    for(int i=0;i<bufSz;i++){
        float eg = pre_eg_val + (egvalVCA - pre_eg_val) / (float) bufSz * i; // linear fade from previous eg value to avoid glitches
        // apply non linearity to filter input
        int16_t warped = ws.Transform(buffer[i]);
        buffer[i] = stmlib::Mix(buffer[i], warped, signature);
        // filter, EG and clip
        data.buf[i*2 + processCh] = fgain * stmlib::SoftClip( eg * filt[ftype]->Process(buffer[i] / 32767.f));
    }
    pre_eg_val = egvalVCA;
    // sync on trigger
    sync[0] = 0;
}

ctagSoundProcessorTBD03::~ctagSoundProcessorTBD03() {
}

const char *ctagSoundProcessorTBD03::GetCStrID() const {
    return id.c_str();
}


void ctagSoundProcessorTBD03::setParamValueInternal(const string& id, const string& key, const int val) {
// autogenerated code here
// sectionCpp0
if(id.compare("trigger") == 0){
	if(key.compare("current") == 0){
		trigger = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_trigger = val;
	}
	return;
}
if(id.compare("sync_trig") == 0){
	if(key.compare("current") == 0){
		sync_trig = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_sync_trig = val;
	}
	return;
}
if(id.compare("pitch") == 0){
	if(key.compare("current") == 0){
		pitch = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_pitch = val;
	}
	return;
}
if(id.compare("shape") == 0){
	if(key.compare("current") == 0){
		shape = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_shape = val;
	}
	return;
}
if(id.compare("param_0") == 0){
	if(key.compare("current") == 0){
		param_0 = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_param_0 = val;
	}
	return;
}
if(id.compare("param_1") == 0){
	if(key.compare("current") == 0){
		param_1 = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_param_1 = val;
	}
	return;
}
if(id.compare("gain") == 0){
	if(key.compare("current") == 0){
		gain = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_gain = val;
	}
	return;
}
if(id.compare("filter_type") == 0){
	if(key.compare("current") == 0){
		filter_type = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_filter_type = val;
	}
	return;
}
if(id.compare("cutoff") == 0){
	if(key.compare("current") == 0){
		cutoff = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_cutoff = val;
	}
	return;
}
if(id.compare("resonance") == 0){
	if(key.compare("current") == 0){
		resonance = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_resonance = val;
	}
	return;
}
if(id.compare("envelope") == 0){
	if(key.compare("current") == 0){
		envelope = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_envelope = val;
	}
	return;
}
if(id.compare("saturation") == 0){
	if(key.compare("current") == 0){
		saturation = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_saturation = val;
	}
	return;
}
if(id.compare("drive") == 0){
	if(key.compare("current") == 0){
		drive = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_drive = val;
	}
	return;
}
if(id.compare("accent") == 0){
	if(key.compare("current") == 0){
		accent = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_accent = val;
	}
	return;
}
if(id.compare("accent_level") == 0){
	if(key.compare("current") == 0){
		accent_level = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_accent_level = val;
	}
	return;
}
if(id.compare("decay_vca") == 0){
	if(key.compare("current") == 0){
		decay_vca = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_decay_vca = val;
	}
	return;
}
if(id.compare("decay_vcf") == 0){
	if(key.compare("current") == 0){
		decay_vcf = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_decay_vcf = val;
	}
	return;
}
if(id.compare("p0_amt") == 0){
	if(key.compare("current") == 0){
		p0_amt = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_p0_amt = val;
	}
	return;
}
if(id.compare("p1_amt") == 0){
	if(key.compare("current") == 0){
		p1_amt = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_p1_amt = val;
	}
	return;
}
// sectionCpp0

}

void ctagSoundProcessorTBD03::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
trigger = model->GetParamValue("trigger", "current");
trig_trigger = model->GetParamValue("trigger", "trig");
sync_trig = model->GetParamValue("sync_trig", "current");
trig_sync_trig = model->GetParamValue("sync_trig", "trig");
pitch = model->GetParamValue("pitch", "current");
cv_pitch = model->GetParamValue("pitch", "cv");
shape = model->GetParamValue("shape", "current");
cv_shape = model->GetParamValue("shape", "cv");
param_0 = model->GetParamValue("param_0", "current");
cv_param_0 = model->GetParamValue("param_0", "cv");
param_1 = model->GetParamValue("param_1", "current");
cv_param_1 = model->GetParamValue("param_1", "cv");
gain = model->GetParamValue("gain", "current");
cv_gain = model->GetParamValue("gain", "cv");
filter_type = model->GetParamValue("filter_type", "current");
cv_filter_type = model->GetParamValue("filter_type", "cv");
cutoff = model->GetParamValue("cutoff", "current");
cv_cutoff = model->GetParamValue("cutoff", "cv");
resonance = model->GetParamValue("resonance", "current");
cv_resonance = model->GetParamValue("resonance", "cv");
envelope = model->GetParamValue("envelope", "current");
cv_envelope = model->GetParamValue("envelope", "cv");
saturation = model->GetParamValue("saturation", "current");
cv_saturation = model->GetParamValue("saturation", "cv");
drive = model->GetParamValue("drive", "current");
cv_drive = model->GetParamValue("drive", "cv");
accent = model->GetParamValue("accent", "current");
trig_accent = model->GetParamValue("accent", "trig");
accent_level = model->GetParamValue("accent_level", "current");
cv_accent_level = model->GetParamValue("accent_level", "cv");
decay_vca = model->GetParamValue("decay_vca", "current");
cv_decay_vca = model->GetParamValue("decay_vca", "cv");
decay_vcf = model->GetParamValue("decay_vcf", "current");
cv_decay_vcf = model->GetParamValue("decay_vcf", "cv");
p0_amt = model->GetParamValue("p0_amt", "current");
cv_p0_amt = model->GetParamValue("p0_amt", "cv");
p1_amt = model->GetParamValue("p1_amt", "current");
cv_p1_amt = model->GetParamValue("p1_amt", "cv");
// sectionCpp1

}
