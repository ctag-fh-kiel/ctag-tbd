#include "ctagSoundProcessorTBDeep.hpp"
#include <iostream>
#include "helpers/ctagFastMath.hpp"
#include "esp_system.h"
#include <cmath>
#include "esp_log.h"
#include "esp_heap_caps.h"

// part of the port is adapted from VCV Rack Audible Instruments, (C) Andrew Belt
using namespace CTAG::SP;

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

static const float kRootScaled[3] = {
        0.125f,
        2.0f,
        130.81f
};

static const tides2::Ratio kRatios[20] = {
        { 0.0625f, 16 },
        { 0.125f, 8 },
        { 0.1666666f, 6 },
        { 0.25f, 4 },
        { 0.3333333f, 3 },
        { 0.5f, 2 },
        { 0.6666666f, 3 },
        { 0.75f, 4 },
        { 0.8f, 5 },
        { 1, 1 },
        { 1, 1 },
        { 1.25f, 4 },
        { 1.3333333f, 3 },
        { 1.5f, 2 },
        { 2.0f, 1 },
        { 3.0f, 1 },
        { 4.0f, 1 },
        { 6.0f, 1 },
        { 8.0f, 1 },
        { 16.0f, 1 },
};

ctagSoundProcessorTBDeep::ctagSoundProcessorTBDeep() {
    setIsStereo();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);

    poly_slope_generator.Init();
    ratio_index_quantizer.Init();
    output_mode = tides2::OUTPUT_MODE_GATES;
    ramp_mode = tides2::RAMP_MODE_LOOPING;
    ramp_extractor.Init(44100.f, 40.f / 44100.f);

    loudAD.SetSampleRate(44100.f);
    loudAD.SetModeExp();
    paramAD.SetSampleRate(44100.f / 32.f);
    paramAD.SetModeLin();
}

void IRAM_ATTR ctagSoundProcessorTBDeep::Process(const ProcessData &data) {
    // ad envelope
    float fAttack = eg_attack / 4095.f * 5.f;
    if(cv_eg_attack != -1){
        fAttack = fabsf(data.cv[cv_eg_attack]) * 5.f;
    }
    loudAD.SetAttack(fAttack);
    paramAD.SetAttack(fAttack);

    float fDecay = eg_decay / 4095.f * 5.f;
    if(cv_eg_decay != -1){
        fDecay = fabsf(data.cv[cv_eg_decay]) * 5.f;
    }
    loudAD.SetDecay(fDecay);
    paramAD.SetDecay(fDecay);

    bool triggerAD = eg_trigger;
    if(trig_eg_trigger != -1){
        triggerAD = data.trig[trig_eg_trigger] != 1;
    }
    if(triggerAD && !eg_pre_trigger){
        loudAD.Trigger();
        paramAD.Trigger();
    }

    eg_pre_trigger = triggerAD;
    bool loopAD = eg_loop;
    if(trig_eg_loop != -1){
        loopAD = data.trig[trig_eg_loop] != 1;
    }
    loudAD.SetLoop(loopAD);
    paramAD.SetLoop(loopAD);

    // sheep setup
    output_mode = (tides2::OutputMode)3; // only mode 3 (tides2::OutputMode)(mode % 4);
    ramp_mode = (tides2::RampMode)1; // only mode 1

    // Input gates
    for(int i=0;i<bufSz;i++){
        if(trig_trigger != -1){
            trig_flags[i] = stmlib::ExtractGateFlags(previous_trig_flag, data.trig[trig_trigger] != 1);
        }else{
            trig_flags[i] = trigger;
        }
        previous_trig_flag = trig_flags[i];

        if(trig_clock != -1){
            clock_flags[i] = stmlib::ExtractGateFlags(previous_clock_flag, data.trig[trig_clock] != 1);
        }else{
            clock_flags[i] = clock;
        }
        previous_clock_flag = clock_flags[i];
    }

    float note = frequency;
    if(cv_frequency != -1){
        note += 12.f * data.cv[cv_frequency] * 5.f;
    }
    CONSTRAIN(note, -96.f, 96.f);

    float eg = paramAD.Process();
    float fm = mod_frequency / 4095.f * 12.f * 5.f;
    if(cv_mod_frequency != -1){
        fm *= data.cv[cv_mod_frequency];
    }else{
        fm *= eg;
    }
    CONSTRAIN(fm, -96.f, 96.f);

    float transposition = note + fm;
    float ramp[tides2::kBlockSize];
    float freq;
    tides2::Range range_mode = tides2::RANGE_AUDIO; // only audio (range < 2) ? tides2::RANGE_CONTROL : tides2::RANGE_AUDIO;

    if (trig_clock != -1) {
        if (must_reset_ramp_extractor) {
            ramp_extractor.Reset();
        }

        tides2::Ratio r = ratio_index_quantizer.Lookup(kRatios, 0.5f + transposition * 0.0105f, 20);
        freq = ramp_extractor.Process(
                range_mode == tides2::RANGE_AUDIO,
                range_mode == tides2::RANGE_AUDIO && ramp_mode == tides2::RAMP_MODE_AR,
                r,
                clock_flags,
                ramp,
                tides2::kBlockSize);
        must_reset_ramp_extractor = false;
    }
    else {
        freq = kRootScaled[2] / 44100.f * stmlib::SemitonesToRatio(transposition);
        must_reset_ramp_extractor = true;
    }

    // Get parameters
    float fSlope = slope / 4095.f;
    float fShape = shape / 4095.f;
    float fSmoothness = smoothness / 4095.f;
    float fShift = shift / 4095.f;

    if(cv_mod_slope != -1){
        fSlope += mod_slope / 4095.f * data.cv[cv_mod_slope];
    }else{
        fSlope += eg * mod_slope / 4095.f;
    }
    CONSTRAIN(fSlope, 0.f, 1.f);
    if(cv_mod_shape != -1){
        fShape += mod_shape / 4095.f * data.cv[cv_mod_shape];
    }else{
        fShape += eg * mod_shape / 4095.f;
    }
    CONSTRAIN(fShape, 0.f, 1.f);
    if(cv_mod_smoothness != -1){
        fSmoothness += mod_smoothness / 4095.f * data.cv[cv_mod_smoothness];
    }else{
        fSmoothness += eg * mod_smoothness / 4095.f;
    }
    CONSTRAIN(fSmoothness, 0.f, 1.f);
    if(cv_shift != -1){
        fShift += mod_shift / 4095.f * data.cv[cv_mod_shift];
    }else{
        fShift += eg * mod_shift / 4095.f;
    }
    CONSTRAIN(fShift, 0.f, 1.f);
    if (output_mode != previous_output_mode) {
        poly_slope_generator.Reset();
        previous_output_mode = output_mode;
    }

    // Render generator
    poly_slope_generator.Render(
            ramp_mode,
            output_mode,
            range_mode,
            freq,
            fSlope,
            fShape,
            fSmoothness,
            fShift,
            trig_flags,
            (trig_trigger == -1) && (trig_clock != -1) ? ramp : NULL,
            out,
            tides2::kBlockSize);

    // which output to route?
    int o0 = out0;
    if(cv_out0 != -1){
        o0 = fabs(data.cv[cv_out0] * 4.f);
        CONSTRAIN(o0, 0, 3);
    }

    int o1 = out1;
    if(cv_out1 != -1){
        o1 = fabs(data.cv[cv_out1] * 4.f);
        CONSTRAIN(o1, 0, 3);
    }

    // loudness
    float fLoud0 = out0_level / 4095.f * 0.25f;
    if(cv_out0_level != -1){
        fLoud0 = data.cv[cv_out0_level];
    }
    float fLoud1 = out1_level / 4095.f;
    if(cv_out1_level != -1){
        fLoud1 = data.cv[cv_out1_level] * 0.25f;
    }
    // loudness modulation
    float fModLevel = mod_level / 4095.f;
    if(cv_mod_level != -1){
        fModLevel = data.cv[cv_mod_level];
    }

    for (int j = 0; j < bufSz; j++) {
        float loud = fModLevel * loudAD.Process();
        if(fModLevel < 0.f) loud -= fModLevel;
        float loud0 = fLoud0 * ((1.f - fabsf(fModLevel)) + loud);
        float loud1 = fLoud1 * ((1.f - fabsf(fModLevel)) + loud);
        data.buf[j * 2] = HELPERS::fasttanh(out[j].channel[o0] * loud0);
        data.buf[j * 2 + 1] = HELPERS::fasttanh(out[j].channel[o1] * loud1);
    }
}

ctagSoundProcessorTBDeep::~ctagSoundProcessorTBDeep() {

}

const char *ctagSoundProcessorTBDeep::GetCStrID() const {
    return id.c_str();
}


void ctagSoundProcessorTBDeep::setParamValueInternal(const string& id, const string& key, const int val) {
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
if(id.compare("clock") == 0){
	if(key.compare("current") == 0){
		clock = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_clock = val;
	}
	return;
}
if(id.compare("out0") == 0){
	if(key.compare("current") == 0){
		out0 = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_out0 = val;
	}
	return;
}
if(id.compare("out0_level") == 0){
	if(key.compare("current") == 0){
		out0_level = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_out0_level = val;
	}
	return;
}
if(id.compare("out1") == 0){
	if(key.compare("current") == 0){
		out1 = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_out1 = val;
	}
	return;
}
if(id.compare("out1_level") == 0){
	if(key.compare("current") == 0){
		out1_level = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_out1_level = val;
	}
	return;
}
if(id.compare("frequency") == 0){
	if(key.compare("current") == 0){
		frequency = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_frequency = val;
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
if(id.compare("slope") == 0){
	if(key.compare("current") == 0){
		slope = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_slope = val;
	}
	return;
}
if(id.compare("smoothness") == 0){
	if(key.compare("current") == 0){
		smoothness = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_smoothness = val;
	}
	return;
}
if(id.compare("shift") == 0){
	if(key.compare("current") == 0){
		shift = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_shift = val;
	}
	return;
}
if(id.compare("eg_trigger") == 0){
	if(key.compare("current") == 0){
		eg_trigger = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_eg_trigger = val;
	}
	return;
}
if(id.compare("eg_loop") == 0){
	if(key.compare("current") == 0){
		eg_loop = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_eg_loop = val;
	}
	return;
}
if(id.compare("eg_attack") == 0){
	if(key.compare("current") == 0){
		eg_attack = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_eg_attack = val;
	}
	return;
}
if(id.compare("eg_decay") == 0){
	if(key.compare("current") == 0){
		eg_decay = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_eg_decay = val;
	}
	return;
}
if(id.compare("mod_level") == 0){
	if(key.compare("current") == 0){
		mod_level = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_level = val;
	}
	return;
}
if(id.compare("mod_frequency") == 0){
	if(key.compare("current") == 0){
		mod_frequency = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_frequency = val;
	}
	return;
}
if(id.compare("mod_shape") == 0){
	if(key.compare("current") == 0){
		mod_shape = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_shape = val;
	}
	return;
}
if(id.compare("mod_slope") == 0){
	if(key.compare("current") == 0){
		mod_slope = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_slope = val;
	}
	return;
}
if(id.compare("mod_smoothness") == 0){
	if(key.compare("current") == 0){
		mod_smoothness = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_smoothness = val;
	}
	return;
}
if(id.compare("mod_shift") == 0){
	if(key.compare("current") == 0){
		mod_shift = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_mod_shift = val;
	}
	return;
}
// sectionCpp0














}

void ctagSoundProcessorTBDeep::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
trigger = model->GetParamValue("trigger", "current");
trig_trigger = model->GetParamValue("trigger", "trig");
clock = model->GetParamValue("clock", "current");
trig_clock = model->GetParamValue("clock", "trig");
out0 = model->GetParamValue("out0", "current");
cv_out0 = model->GetParamValue("out0", "cv");
out0_level = model->GetParamValue("out0_level", "current");
cv_out0_level = model->GetParamValue("out0_level", "cv");
out1 = model->GetParamValue("out1", "current");
cv_out1 = model->GetParamValue("out1", "cv");
out1_level = model->GetParamValue("out1_level", "current");
cv_out1_level = model->GetParamValue("out1_level", "cv");
frequency = model->GetParamValue("frequency", "current");
cv_frequency = model->GetParamValue("frequency", "cv");
shape = model->GetParamValue("shape", "current");
cv_shape = model->GetParamValue("shape", "cv");
slope = model->GetParamValue("slope", "current");
cv_slope = model->GetParamValue("slope", "cv");
smoothness = model->GetParamValue("smoothness", "current");
cv_smoothness = model->GetParamValue("smoothness", "cv");
shift = model->GetParamValue("shift", "current");
cv_shift = model->GetParamValue("shift", "cv");
eg_trigger = model->GetParamValue("eg_trigger", "current");
trig_eg_trigger = model->GetParamValue("eg_trigger", "trig");
eg_loop = model->GetParamValue("eg_loop", "current");
trig_eg_loop = model->GetParamValue("eg_loop", "trig");
eg_attack = model->GetParamValue("eg_attack", "current");
cv_eg_attack = model->GetParamValue("eg_attack", "cv");
eg_decay = model->GetParamValue("eg_decay", "current");
cv_eg_decay = model->GetParamValue("eg_decay", "cv");
mod_level = model->GetParamValue("mod_level", "current");
cv_mod_level = model->GetParamValue("mod_level", "cv");
mod_frequency = model->GetParamValue("mod_frequency", "current");
cv_mod_frequency = model->GetParamValue("mod_frequency", "cv");
mod_shape = model->GetParamValue("mod_shape", "current");
cv_mod_shape = model->GetParamValue("mod_shape", "cv");
mod_slope = model->GetParamValue("mod_slope", "current");
cv_mod_slope = model->GetParamValue("mod_slope", "cv");
mod_smoothness = model->GetParamValue("mod_smoothness", "current");
cv_mod_smoothness = model->GetParamValue("mod_smoothness", "cv");
mod_shift = model->GetParamValue("mod_shift", "current");
cv_mod_shift = model->GetParamValue("mod_shift", "cv");
// sectionCpp1














}
