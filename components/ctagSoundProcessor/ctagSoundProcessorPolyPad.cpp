#include "ctagSoundProcessorPolyPad.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "esp_system.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

ctagSoundProcessorPolyPad::ctagSoundProcessorPolyPad()
{
    setIsStereo();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);

    quantizer.Init();
}

void ctagSoundProcessorPolyPad::Process(const ProcessData &data) {
    // zero input
    for(int i=0;i<32;i++){
        data.buf[i*2 + processCh] = 0.f;
    }

    // kill unused chords, ugly but effective std c++, i think there's a better way in c++20
    v_voices.erase(
            std::remove_if(v_voices.begin(), v_voices.end(),
                           [&](ChordSynth &v) { return v.IsDead(); }
            ),
            v_voices.end());

    // start chord
    bool shouldTrigger = enableEG;
    if(trig_enableEG != -1) shouldTrigger = data.trig[trig_enableEG] == 1 ? 0 : 1; // inverted logic
    if(latchEG){
        if(!toggle && shouldTrigger){
            latched = !latched;
            toggle = true;
        }else if(!shouldTrigger){
            toggle = false;
        }
        if(latched && shouldTrigger){
            shouldTrigger = false;
        }
    }else{
        latched = true;
        toggle = false;
    }
    shouldTrigger = shouldTrigger && (latchVoice == false);

    // start processing voices
    if (shouldTrigger) {
        // check if voice needs to be killed because too many are active
        if(v_voices.size() > ncvoices){
            // sort vector according to voice time to live
            sort(begin(v_voices), end(v_voices),
                 [](ChordSynth& a, ChordSynth& b){return a.GetTTL() > b.GetTTL();}
                 );
            // kill the one which is most quiet = shortest TTL
            v_voices.erase(v_voices.end() - 1);
        }

        // hold voices
        bool shouldHold = voicehold;
        if(trig_voicehold != -1){shouldHold = data.trig[trig_voicehold] == 0 ? 1:0;} // inverted logic
        if(shouldHold){
            for(auto &v:v_voices){
                v.Hold();
            }
        }

        // start new voice with current parameter settings including cv mod capture
        ChordSynth::ChordParams params;

        // pitch calculation and quantization + fm
        params.pitch = pitch;
        if(cv_pitch != -1){params.pitch += static_cast<int16_t>(data.cv[cv_pitch] * 5.f * 12.f * 128.f);}
        int32_t sc = q_scale;
        if(cv_q_scale != -1){
            sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale])* 48.f);
        }
        CONSTRAIN(sc, 0, 47);
        quantizer.Configure(braids::scales[sc]);
        params.pitch = quantizer.Process(params.pitch, pitch);
        CONSTRAIN(params.pitch, 0, 16383);

        // which chord
        params.chord = chord;
        if(cv_chord != -1){params.chord = static_cast<int16_t>(fabsf(data.cv[cv_chord]) * kChordNumChords);}
        CONSTRAIN(params.chord, 0, kChordNumChords-1)
        params.nnotes = nnotes;
        if(cv_nnotes != -1){params.nnotes = static_cast<int16_t>(fabsf(data.cv[cv_nnotes]) * 4.f) + 1;}
        CONSTRAIN(params.nnotes, 1, 4)

        params.detune = detune;
        if(cv_detune != -1){params.detune = static_cast<int16_t>(fabsf(data.cv[cv_detune]) * 32767.f);}
        CONSTRAIN(params.detune, 0, 32767)
        params.inversion = inversion;
        if(cv_inversion != -1){params.inversion = static_cast<int16_t>(fabsf(data.cv[cv_inversion]) * 6.f - 3.f);}
        CONSTRAIN(params.inversion, -2, 2)
        float maxA, maxD, maxR;
        if(eg_slow_fast){
            maxA = 60.f; maxD = 40.f; maxR = 40.f;
        }else{
            maxA = 10.f; maxD = 10.f; maxR = 10.f;
        }
        params.attack = static_cast<float>(attack) / 4095.f * maxA;
        if(cv_attack != -1){params.attack = fabsf(data.cv[cv_attack]) * maxA;}
        CONSTRAIN(params.attack, 0.f, maxA)
        params.decay = static_cast<float>(decay) / 4095.f * maxD;
        if(cv_decay != -1){params.decay = fabsf(data.cv[cv_decay]) * maxD;}
        CONSTRAIN(params.decay, 0.f, maxD)
        params.sustain = static_cast<float>(sustain) / 4095.f;
        if(cv_sustain != -1){params.sustain = fabsf(data.cv[cv_sustain]);}
        CONSTRAIN(params.sustain, 0.f, 1.f)
        params.release = static_cast<float>(release) / 4095.f * maxR;
        if(cv_release != -1){params.release = fabsf(data.cv[cv_release]) * maxR;}
        CONSTRAIN(params.release, 0.f, maxR)

        // vibrato
        params.lfo1_freq = static_cast<float>(lfo1_freq) / 4095.f * 5.f;
        if(cv_lfo1_freq != -1){params.lfo1_freq = fabsf(data.cv[cv_lfo1_freq]) * 5.f;}
        CONSTRAIN(params.lfo1_freq, 0.f, 5.f)
        params.lfo1_amt = static_cast<float>(lfo1_amt) / 4095.f * 5.f;
        if(cv_lfo1_amt != -1){params.lfo1_amt = fabsf(data.cv[cv_lfo1_amt]) * 5.f;}
        CONSTRAIN(params.lfo1_amt, 0.f, 5.f)

        // filter fm chopper
        params.lfo2_freq = static_cast<float>(lfo2_freq) / 4095.f * 5.f;
        if(cv_lfo2_freq != -1){params.lfo2_freq = fabsf(data.cv[cv_lfo2_freq]) * 5.f;}
        CONSTRAIN(params.lfo2_freq, 0.f, 5.f)
        params.lfo2_amt = static_cast<float>(lfo2_amt) / 4095.f;
        if(cv_lfo2_amt != -1){params.lfo2_amt = fabsf(data.cv[cv_lfo2_amt]);}
        CONSTRAIN(params.lfo2_amt, 0.f, 1.f)
        params.lfo2_random_phase = lfo2_rphase;
        params.eg_filt_amt = static_cast<float>(eg_filt_amt) / 4095.f;
        if(cv_eg_filt_amt != -1){params.eg_filt_amt = data.cv[cv_eg_filt_amt];}
        CONSTRAIN(params.eg_filt_amt, -1.f, 1.f)
        params.filter_type = filter_type;
        if(cv_filter_type != -1){params.filter_type = fabsf(data.cv[cv_filter_type]) * 3.f;}
        CONSTRAIN(params.filter_type, 0, 2)
        //printf("%f, %f, %f, %f\n", data.cv[0], data.cv[1], data.cv[2], data.cv[3]);
        //PrintParams(params);
        // make voice, constructor triggers playback
        ChordSynth voice(params);
        v_voices.push_back(voice);

        latchVoice = true;
    }

    // render buffers with updated cutoff, resonance and detune
    uint32_t c = cutoff;
    if(cv_cutoff != -1){
        c = static_cast<int32_t>(1750.f + fabsf(data.cv[cv_cutoff]) * (16384.f - 1750.f));
        CONSTRAIN(c, 1750, 16384)
    }
    uint32_t r = resonance;
    if(cv_resonance != -1){
        r = static_cast<int32_t>(fabsf(data.cv[cv_resonance]) * 32767.f);
        CONSTRAIN(r, 0, 32767)
    }
    uint32_t d = detune;
    if(cv_detune != -1){
        d = static_cast<int32_t>(fabsf(data.cv[cv_detune]) * 32767.f);
        CONSTRAIN(d, 0, 32767)
    }

    for (auto &v:v_voices) {
        v.SetCutoff(c);
        v.SetResonance(r);
        v.SetDetune(d);
        v.Process(data.buf, processCh);
    }

    // apply gain
    float fGain = gain / 4095.f * 2.f;
    if(cv_gain != -1){
        fGain = fabsf(data.cv[cv_gain]) * 2.f;
    }
    for(int i=0;i<bufSz;i++){
        data.buf[i*2 + processCh] *= fGain;
    }

    // note off including latched mode
    bool shouldNoteOff = !enableEG;
    if(trig_enableEG != -1) shouldNoteOff = data.trig[trig_enableEG]; // already inverted
    if(latchEG){
        shouldNoteOff = !shouldNoteOff;
        if(!latched && shouldNoteOff)
            shouldNoteOff = false;
    }
    shouldNoteOff = shouldNoteOff && (latchVoice == true);

    if (shouldNoteOff) {
        for (auto &v:v_voices) {
            v.NoteOff();
        }
        latchVoice = false;
    }

}

ctagSoundProcessorPolyPad::~ctagSoundProcessorPolyPad() {
}

const char *ctagSoundProcessorPolyPad::GetCStrID() const {
    return id.c_str();
}


void ctagSoundProcessorPolyPad::setParamValueInternal(const string &id, const string &key, const int val) {
// autogenerated code here
// sectionCpp0
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
if(id.compare("q_scale") == 0){
	if(key.compare("current") == 0){
		q_scale = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_q_scale = val;
	}
	return;
}
if(id.compare("chord") == 0){
	if(key.compare("current") == 0){
		chord = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_chord = val;
	}
	return;
}
if(id.compare("inversion") == 0){
	if(key.compare("current") == 0){
		inversion = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_inversion = val;
	}
	return;
}
if(id.compare("detune") == 0){
	if(key.compare("current") == 0){
		detune = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_detune = val;
	}
	return;
}
if(id.compare("nnotes") == 0){
	if(key.compare("current") == 0){
		nnotes = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_nnotes = val;
	}
	return;
}
if(id.compare("ncvoices") == 0){
	if(key.compare("current") == 0){
		ncvoices = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_ncvoices = val;
	}
	return;
}
if(id.compare("voicehold") == 0){
	if(key.compare("current") == 0){
		voicehold = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_voicehold = val;
	}
	return;
}
if(id.compare("lfo1_freq") == 0){
	if(key.compare("current") == 0){
		lfo1_freq = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_lfo1_freq = val;
	}
	return;
}
if(id.compare("lfo1_amt") == 0){
	if(key.compare("current") == 0){
		lfo1_amt = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_lfo1_amt = val;
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
if(id.compare("lfo2_freq") == 0){
	if(key.compare("current") == 0){
		lfo2_freq = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_lfo2_freq = val;
	}
	return;
}
if(id.compare("lfo2_amt") == 0){
	if(key.compare("current") == 0){
		lfo2_amt = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_lfo2_amt = val;
	}
	return;
}
if(id.compare("lfo2_rphase") == 0){
	if(key.compare("current") == 0){
		lfo2_rphase = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_lfo2_rphase = val;
	}
	return;
}
if(id.compare("eg_filt_amt") == 0){
	if(key.compare("current") == 0){
		eg_filt_amt = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_eg_filt_amt = val;
	}
	return;
}
if(id.compare("enableEG") == 0){
	if(key.compare("current") == 0){
		enableEG = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_enableEG = val;
	}
	return;
}
if(id.compare("latchEG") == 0){
	if(key.compare("current") == 0){
		latchEG = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_latchEG = val;
	}
	return;
}
if(id.compare("eg_slow_fast") == 0){
	if(key.compare("current") == 0){
		eg_slow_fast = val;
		return;
	}else if(key.compare("trig") == 0){
		if(val >= -1 && val <= 1)
			trig_eg_slow_fast = val;
	}
	return;
}
if(id.compare("attack") == 0){
	if(key.compare("current") == 0){
		attack = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_attack = val;
	}
	return;
}
if(id.compare("decay") == 0){
	if(key.compare("current") == 0){
		decay = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_decay = val;
	}
	return;
}
if(id.compare("sustain") == 0){
	if(key.compare("current") == 0){
		sustain = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_sustain = val;
	}
	return;
}
if(id.compare("release") == 0){
	if(key.compare("current") == 0){
		release = val;
		return;
	}else if(key.compare("cv") == 0){
		if(val >= -1 && val <= 3)
			cv_release = val;
	}
	return;
}
// sectionCpp0




}

void ctagSoundProcessorPolyPad::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
gain = model->GetParamValue("gain", "current");
cv_gain = model->GetParamValue("gain", "cv");
pitch = model->GetParamValue("pitch", "current");
cv_pitch = model->GetParamValue("pitch", "cv");
q_scale = model->GetParamValue("q_scale", "current");
cv_q_scale = model->GetParamValue("q_scale", "cv");
chord = model->GetParamValue("chord", "current");
cv_chord = model->GetParamValue("chord", "cv");
inversion = model->GetParamValue("inversion", "current");
cv_inversion = model->GetParamValue("inversion", "cv");
detune = model->GetParamValue("detune", "current");
cv_detune = model->GetParamValue("detune", "cv");
nnotes = model->GetParamValue("nnotes", "current");
cv_nnotes = model->GetParamValue("nnotes", "cv");
ncvoices = model->GetParamValue("ncvoices", "current");
cv_ncvoices = model->GetParamValue("ncvoices", "cv");
voicehold = model->GetParamValue("voicehold", "current");
trig_voicehold = model->GetParamValue("voicehold", "trig");
lfo1_freq = model->GetParamValue("lfo1_freq", "current");
cv_lfo1_freq = model->GetParamValue("lfo1_freq", "cv");
lfo1_amt = model->GetParamValue("lfo1_amt", "current");
cv_lfo1_amt = model->GetParamValue("lfo1_amt", "cv");
filter_type = model->GetParamValue("filter_type", "current");
cv_filter_type = model->GetParamValue("filter_type", "cv");
cutoff = model->GetParamValue("cutoff", "current");
cv_cutoff = model->GetParamValue("cutoff", "cv");
resonance = model->GetParamValue("resonance", "current");
cv_resonance = model->GetParamValue("resonance", "cv");
lfo2_freq = model->GetParamValue("lfo2_freq", "current");
cv_lfo2_freq = model->GetParamValue("lfo2_freq", "cv");
lfo2_amt = model->GetParamValue("lfo2_amt", "current");
cv_lfo2_amt = model->GetParamValue("lfo2_amt", "cv");
lfo2_rphase = model->GetParamValue("lfo2_rphase", "current");
trig_lfo2_rphase = model->GetParamValue("lfo2_rphase", "trig");
eg_filt_amt = model->GetParamValue("eg_filt_amt", "current");
cv_eg_filt_amt = model->GetParamValue("eg_filt_amt", "cv");
enableEG = model->GetParamValue("enableEG", "current");
trig_enableEG = model->GetParamValue("enableEG", "trig");
latchEG = model->GetParamValue("latchEG", "current");
trig_latchEG = model->GetParamValue("latchEG", "trig");
eg_slow_fast = model->GetParamValue("eg_slow_fast", "current");
trig_eg_slow_fast = model->GetParamValue("eg_slow_fast", "trig");
attack = model->GetParamValue("attack", "current");
cv_attack = model->GetParamValue("attack", "cv");
decay = model->GetParamValue("decay", "current");
cv_decay = model->GetParamValue("decay", "cv");
sustain = model->GetParamValue("sustain", "current");
cv_sustain = model->GetParamValue("sustain", "cv");
release = model->GetParamValue("release", "current");
cv_release = model->GetParamValue("release", "cv");
// sectionCpp1




}

void ctagSoundProcessorPolyPad::PrintParams(ChordSynth::ChordParams &params) {
    printf("pitch %d\n", params.pitch);
    printf("chord %d\n", params.chord);
    printf("nnotes %d\n", params.nnotes);
    printf("inversion %d\n", params.inversion);
    printf("detune %d\n", params.detune);
    printf("attack %f, decay %f, release %f, sustain %f\n", params.attack, params.decay, params.release, params.sustain);
    printf("lfo1_freq %f, lfo2_freq %f, lfo1_amt %f, lfo2_amt %f\n",params.lfo1_freq, params.lfo2_freq, params.lfo1_amt, params.lfo2_amt);
    printf("eg_filt_amt %f\n",params.eg_filt_amt);
    printf("filter_freq %d, filter_reso %d, filter_type %d\n",params.filter_freq, params.filter_reso, params.filter_type);
}
