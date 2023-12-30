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

#include "ctagSoundProcessorPolyPad.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>
#include "esp_system.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

void ctagSoundProcessorPolyPad::Init() {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    quantizer.Init();
}

void ctagSoundProcessorPolyPad::Process(const ProcessData &data) {
    // zero input
    for (int i = 0; i < 32; i++) {
        data.buf[i * 2 + processCh] = 0.f;
    }

    // kill unused chords, ugly but effective std c++, i think there's a better way in c++20
    v_voices.erase(
            std::remove_if(v_voices.begin(), v_voices.end(),
                           [&](ChordSynth &v) { return v.IsDead(); }
            ),
            v_voices.end());

    // start chord
    bool shouldTrigger = enableEG;
    if (trig_enableEG != -1) shouldTrigger = data.trig[trig_enableEG] == 1 ? 0 : 1; // inverted logic
    if (latchEG) {
        if (!toggle && shouldTrigger) {
            latched = !latched;
            toggle = true;
        } else if (!shouldTrigger) {
            toggle = false;
        }
        if (latched && shouldTrigger) {
            shouldTrigger = false;
        }
    } else {
        latched = true;
        toggle = false;
    }
    shouldTrigger = shouldTrigger && (latchVoice == false);
    // start processing voices
    if (shouldTrigger) {
        // check if voice needs to be killed because too many are active
        if (v_voices.size() > ncvoices) {
            // sort vector according to voice time to live
            sort(begin(v_voices), end(v_voices),
                 [](ChordSynth &a, ChordSynth &b) { return a.GetTTL() > b.GetTTL(); }
            );
            // kill the one which is most quiet = shortest TTL
            v_voices.erase(v_voices.end() - 1);
        }

        // hold voices
        bool shouldHold = voicehold;
        if (trig_voicehold != -1) { shouldHold = data.trig[trig_voicehold] == 0 ? 1 : 0; } // inverted logic
        if (shouldHold) {
            for (auto &v:v_voices) {
                v.Hold();
            }
        }

        // start new voice with current parameter settings including cv mod capture
        ChordSynth::ChordParams params;

        // pitch calculation and quantization + fm
        params.pitch = pitch;
        if (cv_pitch != -1) { params.pitch += static_cast<int16_t>(data.cv[cv_pitch] * 5.f * 12.f * 128.f); }
        int32_t sc = q_scale;
        if (cv_q_scale != -1) {
            sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
        }
        CONSTRAIN(sc, 0, 47);
        quantizer.Configure(braids::scales[sc]);
        params.pitch = quantizer.Process(params.pitch, pitch);
        CONSTRAIN(params.pitch, 0, 16383);

        // which chord
        params.chord = chord;
        if (cv_chord != -1) { params.chord = static_cast<int16_t>(fabsf(data.cv[cv_chord]) * kChordNumChords); }
        CONSTRAIN(params.chord, 0, kChordNumChords - 1)
        params.nnotes = nnotes;
        if (cv_nnotes != -1) { params.nnotes = static_cast<int16_t>(fabsf(data.cv[cv_nnotes]) * 4.f) + 1; }
        CONSTRAIN(params.nnotes, 1, 4)

        params.detune = detune;
        if (cv_detune != -1) { params.detune = static_cast<int16_t>(fabsf(data.cv[cv_detune]) * 32767.f); }
        CONSTRAIN(params.detune, 0, 32767)
        params.inversion = inversion;
        if (cv_inversion != -1) { params.inversion = static_cast<int16_t>(fabsf(data.cv[cv_inversion]) * 6.f - 3.f); }
        CONSTRAIN(params.inversion, -2, 2)
        float maxA, maxD, maxR;
        if (eg_slow_fast) {
            maxA = 60.f;
            maxD = 40.f;
            maxR = 40.f;
        } else {
            maxA = 10.f;
            maxD = 10.f;
            maxR = 10.f;
        }
        params.attack = static_cast<float>(attack) / 4095.f * maxA;
        if (cv_attack != -1) { params.attack = fabsf(data.cv[cv_attack]) * maxA; }
        CONSTRAIN(params.attack, 0.f, maxA)
        params.decay = static_cast<float>(decay) / 4095.f * maxD;
        if (cv_decay != -1) { params.decay = fabsf(data.cv[cv_decay]) * maxD; }
        CONSTRAIN(params.decay, 0.f, maxD)
        params.sustain = static_cast<float>(sustain) / 4095.f;
        if (cv_sustain != -1) { params.sustain = fabsf(data.cv[cv_sustain]); }
        CONSTRAIN(params.sustain, 0.f, 1.f)
        params.release = static_cast<float>(release) / 4095.f * maxR;
        if (cv_release != -1) { params.release = fabsf(data.cv[cv_release]) * maxR; }
        CONSTRAIN(params.release, 0.f, maxR)

        // vibrato
        params.lfo1_freq = static_cast<float>(lfo1_freq) / 4095.f * 5.f;
        if (cv_lfo1_freq != -1) { params.lfo1_freq = fabsf(data.cv[cv_lfo1_freq]) * 5.f; }
        CONSTRAIN(params.lfo1_freq, 0.f, 5.f)
        params.lfo1_amt = static_cast<float>(lfo1_amt) / 4095.f * 5.f;
        if (cv_lfo1_amt != -1) { params.lfo1_amt = fabsf(data.cv[cv_lfo1_amt]) * 5.f; }
        CONSTRAIN(params.lfo1_amt, 0.f, 5.f)

        // filter fm chopper
        params.lfo2_freq = static_cast<float>(lfo2_freq) / 4095.f * 5.f;
        if (cv_lfo2_freq != -1) { params.lfo2_freq = fabsf(data.cv[cv_lfo2_freq]) * 5.f; }
        CONSTRAIN(params.lfo2_freq, 0.f, 5.f)
        params.lfo2_amt = static_cast<float>(lfo2_amt) / 4095.f;
        if (cv_lfo2_amt != -1) { params.lfo2_amt = fabsf(data.cv[cv_lfo2_amt]); }
        CONSTRAIN(params.lfo2_amt, 0.f, 1.f)
        params.lfo2_random_phase = lfo2_rphase;
        params.eg_filt_amt = static_cast<float>(eg_filt_amt) / 4095.f;
        if (cv_eg_filt_amt != -1) { params.eg_filt_amt = data.cv[cv_eg_filt_amt]; }
        CONSTRAIN(params.eg_filt_amt, -1.f, 1.f)
        params.filter_type = filter_type;
        if (cv_filter_type != -1) { params.filter_type = fabsf(data.cv[cv_filter_type]) * 3.f; }
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
    if (cv_cutoff != -1) {
        c = static_cast<int32_t>(1750.f + fabsf(data.cv[cv_cutoff]) * (16384.f - 1750.f));
        CONSTRAIN(c, 1750, 16384)
    }
    int32_t r = resonance;
    if (cv_resonance != -1) {
        r = static_cast<int32_t>(fabsf(data.cv[cv_resonance]) * 32767.f);
        CONSTRAIN(r, 0, 32767)
    }
    int32_t d = detune;
    if (cv_detune != -1) {
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
    if (cv_gain != -1) {
        fGain = fabsf(data.cv[cv_gain]) * 2.f;
    }
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] *= fGain;
    }

    // note off including latched mode
    bool shouldNoteOff = !enableEG;
    if (trig_enableEG != -1) shouldNoteOff = data.trig[trig_enableEG]; // already inverted
    if (latchEG) {
        shouldNoteOff = !shouldNoteOff;
        if (!latched && shouldNoteOff)
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

void ctagSoundProcessorPolyPad::PrintParams(ChordSynth::ChordParams &params) {
    printf("pitch %d\n", params.pitch);
    printf("chord %d\n", params.chord);
    printf("nnotes %d\n", params.nnotes);
    printf("inversion %d\n", params.inversion);
    printf("detune %d\n", params.detune);
    printf("attack %f, decay %f, release %f, sustain %f\n", params.attack, params.decay, params.release,
           params.sustain);
    printf("lfo1_freq %f, lfo2_freq %f, lfo1_amt %f, lfo2_amt %f\n", params.lfo1_freq, params.lfo2_freq,
           params.lfo1_amt, params.lfo2_amt);
    printf("eg_filt_amt %f\n", params.eg_filt_amt);
    printf("filter_freq %d, filter_reso %d, filter_type %d\n", params.filter_freq, params.filter_reso,
           params.filter_type);
}

void ctagSoundProcessorPolyPad::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("gain", [&](const int val) { gain = val; });
    pMapCv.emplace("gain", [&](const int val) { cv_gain = val; });
    pMapPar.emplace("pitch", [&](const int val) { pitch = val; });
    pMapCv.emplace("pitch", [&](const int val) { cv_pitch = val; });
    pMapPar.emplace("q_scale", [&](const int val) { q_scale = val; });
    pMapCv.emplace("q_scale", [&](const int val) { cv_q_scale = val; });
    pMapPar.emplace("chord", [&](const int val) { chord = val; });
    pMapCv.emplace("chord", [&](const int val) { cv_chord = val; });
    pMapPar.emplace("inversion", [&](const int val) { inversion = val; });
    pMapCv.emplace("inversion", [&](const int val) { cv_inversion = val; });
    pMapPar.emplace("detune", [&](const int val) { detune = val; });
    pMapCv.emplace("detune", [&](const int val) { cv_detune = val; });
    pMapPar.emplace("nnotes", [&](const int val) { nnotes = val; });
    pMapCv.emplace("nnotes", [&](const int val) { cv_nnotes = val; });
    pMapPar.emplace("ncvoices", [&](const int val) { ncvoices = val; });
    pMapCv.emplace("ncvoices", [&](const int val) { cv_ncvoices = val; });
    pMapPar.emplace("voicehold", [&](const int val) { voicehold = val; });
    pMapTrig.emplace("voicehold", [&](const int val) { trig_voicehold = val; });
    pMapPar.emplace("lfo1_freq", [&](const int val) { lfo1_freq = val; });
    pMapCv.emplace("lfo1_freq", [&](const int val) { cv_lfo1_freq = val; });
    pMapPar.emplace("lfo1_amt", [&](const int val) { lfo1_amt = val; });
    pMapCv.emplace("lfo1_amt", [&](const int val) { cv_lfo1_amt = val; });
    pMapPar.emplace("filter_type", [&](const int val) { filter_type = val; });
    pMapCv.emplace("filter_type", [&](const int val) { cv_filter_type = val; });
    pMapPar.emplace("cutoff", [&](const int val) { cutoff = val; });
    pMapCv.emplace("cutoff", [&](const int val) { cv_cutoff = val; });
    pMapPar.emplace("resonance", [&](const int val) { resonance = val; });
    pMapCv.emplace("resonance", [&](const int val) { cv_resonance = val; });
    pMapPar.emplace("lfo2_freq", [&](const int val) { lfo2_freq = val; });
    pMapCv.emplace("lfo2_freq", [&](const int val) { cv_lfo2_freq = val; });
    pMapPar.emplace("lfo2_amt", [&](const int val) { lfo2_amt = val; });
    pMapCv.emplace("lfo2_amt", [&](const int val) { cv_lfo2_amt = val; });
    pMapPar.emplace("lfo2_rphase", [&](const int val) { lfo2_rphase = val; });
    pMapTrig.emplace("lfo2_rphase", [&](const int val) { trig_lfo2_rphase = val; });
    pMapPar.emplace("eg_filt_amt", [&](const int val) { eg_filt_amt = val; });
    pMapCv.emplace("eg_filt_amt", [&](const int val) { cv_eg_filt_amt = val; });
    pMapPar.emplace("enableEG", [&](const int val) { enableEG = val; });
    pMapTrig.emplace("enableEG", [&](const int val) { trig_enableEG = val; });
    pMapPar.emplace("latchEG", [&](const int val) { latchEG = val; });
    pMapTrig.emplace("latchEG", [&](const int val) { trig_latchEG = val; });
    pMapPar.emplace("eg_slow_fast", [&](const int val) { eg_slow_fast = val; });
    pMapTrig.emplace("eg_slow_fast", [&](const int val) { trig_eg_slow_fast = val; });
    pMapPar.emplace("attack", [&](const int val) { attack = val; });
    pMapCv.emplace("attack", [&](const int val) { cv_attack = val; });
    pMapPar.emplace("decay", [&](const int val) { decay = val; });
    pMapCv.emplace("decay", [&](const int val) { cv_decay = val; });
    pMapPar.emplace("sustain", [&](const int val) { sustain = val; });
    pMapCv.emplace("sustain", [&](const int val) { cv_sustain = val; });
    pMapPar.emplace("release", [&](const int val) { release = val; });
    pMapCv.emplace("release", [&](const int val) { cv_release = val; });
    isStereo = false;
    id = "PolyPad";
    // sectionCpp0
}
