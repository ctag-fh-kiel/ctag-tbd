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
#include "RackPolyPad.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

void RackPolyPad::Init(const PickSeqRackInitData *initdata) {
    // uint8_t *privatedata = initdata->allocator(1000);

    for(auto &s:pp_v_voices){
        s.Reset();
    }
    pp_quantizer.Init();

    initdata->rack->registerParamAndCC(initdata, "chord", 8, [&](const int val) { pp_chord = val; });
    initdata->rack->registerParamAndCC(initdata, "inversion", 9, [&](const int val) { pp_inversion = val; });
    initdata->rack->registerParamAndCC(initdata, "detune", 10, [&](const int val) { pp_detune = val; });
    // initdata->rack->registerParamAndCC(initdata, "pitch", 9, [&](const int val) { pp_pitch = val; });

    initdata->rack->registerParamAndCC(initdata, "cutoff", 11, [&](const int val) { pp_cutoff = val; });
    initdata->rack->registerParamAndCC(initdata, "resonance", 12, [&](const int val) { pp_resonance = val; });
    initdata->rack->registerParamAndCC(initdata, "filter_type", 13, [&](const int val) { pp_filter_type = val; });
    initdata->rack->registerParamAndCC(initdata, "q_scale", 14, [&](const int val) { pp_q_scale = val; });

    initdata->rack->registerParamAndCC(initdata, "attack", 15, [&](const int val) { pp_attack = val; });
    initdata->rack->registerParamAndCC(initdata, "decay", 16, [&](const int val) { pp_decay = val; });
    initdata->rack->registerParamAndCC(initdata, "sustain", 17, [&](const int val) { pp_sustain = val; });
    initdata->rack->registerParamAndCC(initdata, "release", 18, [&](const int val) { pp_release = val; });

    initdata->rack->registerParamAndCC(initdata, "lfo1_freq", 19, [&](const int val) { pp_lfo1_freq = val; });
    initdata->rack->registerParamAndCC(initdata, "lfo1_amt", 20, [&](const int val) { pp_lfo1_amt = val; });
    initdata->rack->registerParamAndCC(initdata, "lfo2_freq", 21, [&](const int val) { pp_lfo2_freq = val; });
    initdata->rack->registerParamAndCC(initdata, "lfo2_amt", 22, [&](const int val) { pp_lfo2_amt = val; });

    initdata->rack->registerParamAndCC(initdata, "eg_filt_amt", 23, [&](const int val) { pp_eg_filt_amt = val; }); // not used
    // initdata->rack->registerParamAndCC(initdata, "eg_slow_fast", 23, [&](const int val) { pp_eg_slow_fast = val; });
    initdata->rack->registerParamAndCC(initdata, "lfo2_rphase", 24, [&](const int val) { pp_lfo2_rphase = val; }); // not used

    initdata->rack->registerParamAndCC(initdata, "nnotes", 25, [&](const int val) { pp_nnotes = val; });
    // initdata->rack->registerParamAndCC(initdata, "ncvoices", 26, [&](const int val) { pp_ncvoices = val; });
    // initdata->rack->registerParamAndCC(initdata, "voicehold", 27, [&](const int val) { pp_voicehold = val; });
    // initdata->rack->registerParamAndCC(initdata, "latchEG", 28, [&](const int val) { pp_latchEG = val; });

    this->enabled = false;
    // pp_preNCVoices = 99;
};

void RackPolyPad::noteOn(uint8_t note, uint8_t vel) {
    // TODO: Implement
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
}

void RackPolyPad::noteOff(uint8_t note, uint8_t vel) {
    // TODO: Implement
}

void RackPolyPad::Process(const PicoSeqRackProcessData &data) {
    // std::fill_n(pp_out, BUF_SZ, 0.f);
    if (!this->enabled) {
        return;
    }

    std::fill_n(pp_out_stereo, BUF_SZ * 2, 0.f);

    int32_t NCVoices = 1; // pp_ncvoices;
    // CONSTRAIN(NCVoices, 1, 8)
    // if(pp_preNCVoices != NCVoices){
    //     for(auto &s:pp_v_voices){
    //         s.Reset();
    //     }
    //     pp_preNCVoices = NCVoices;
    // }

    // start chord
    bool shouldTrigger = false; // pp_enableEG;
    if (midi_trig) {
        shouldTrigger = true;
        midi_trig = false;
    }
    // if (trig_pp_enableEG != -1) shouldTrigger = data.trig[trig_pp_enableEG] == 1 ? 0 : 1; // inverted logic
    if (shouldTrigger != trig_prev) {
        // if (shouldTrigger) {
        //     printf("PP1\n");
        // }
        trig_prev = shouldTrigger;
    } else {
        shouldTrigger = false;
    }

    // if (pp_latchEG) {
    //     if (!pp_toggle && shouldTrigger) {
    //         pp_latched = !pp_latched;
    //         pp_toggle = true;
    //     } else if (!shouldTrigger) {
    //         pp_toggle = false;
    //     }
    //     if (pp_latched && shouldTrigger) {
    //         shouldTrigger = false;
    //     }
    // } else {
    //     pp_latched = true;
    //     pp_toggle = false;
    // }
    // shouldTrigger = shouldTrigger && (pp_latchVoice == false);
    // start processing voices

    if (shouldTrigger) {
        // check if voice needs to be killed because too many are active

        // sort array according to voice time to live, last in array has shortest TTL
        // sort(begin(pp_v_voices), end(pp_v_voices) - (8-NCVoices),
        //      [](ChordSynth &a, ChordSynth &b) { return a.GetTTL() > b.GetTTL(); }
        // );

        // hold voices
        // bool shouldHold = pp_voicehold;
        // // if (trig_pp_voicehold != -1) { shouldHold = data.trig[trig_pp_voicehold] == 0 ? 1 : 0; } // inverted logic
        // if (shouldHold) {
        //     for (int i=0;i<NCVoices-1;i++) {
        //         if(!pp_v_voices[i].IsDead())
        //             pp_v_voices[i].Hold();
        //     }
        // }

        // start new voice with current parameter settings including cv mod capture
        ChordSynth::ChordParams params;

        // pitch calculation and quantization + fm
        params.pitch = midi_note * 128;
        // if (cv_pp_pitch != -1) { params.pitch += static_cast<int16_t>(data.cv[cv_pp_pitch] * 5.f * 12.f * 128.f); }
        // int32_t sc = pp_q_scale * 48 / 4096;
        // if (cv_pp_q_scale != -1) {
        //     sc = static_cast<int32_t>(fabsf(data.cv[cv_pp_q_scale]) * 48.f);
        // }
        // CONSTRAIN(sc, 0, 47);
        // pp_quantizer.Configure(braids::scales[sc]);
        // params.pitch = pp_quantizer.Process(params.pitch, midi_freq);
        CONSTRAIN(params.pitch, 0, 16383);

        // which chord
        params.chord = pp_chord * kChordNumChords  / 4096;
        // if (cv_pp_chord != -1) { params.chord = static_cast<int16_t>(fabsf(data.cv[cv_pp_chord]) * kChordNumChords); }
        CONSTRAIN(params.chord, 0, kChordNumChords - 1)

        params.nnotes = 1 + (pp_nnotes * 4 / 4096);
        // if (cv_pp_nnotes != -1) { params.nnotes = static_cast<int16_t>(fabsf(data.cv[cv_pp_nnotes]) * 4.f) + 1; }
        CONSTRAIN(params.nnotes, 1, 4)

        params.detune = pp_detune * 4;
        // if (cv_pp_detune != -1) { params.detune = static_cast<int16_t>(fabsf(data.cv[cv_pp_detune]) * 32767.f); }
        CONSTRAIN(params.detune, 0, 32767)
        params.inversion = pp_inversion * 2 / 4096;
        // if (cv_pp_inversion != -1) { params.inversion = static_cast<int16_t>(fabsf(data.cv[cv_pp_inversion]) * 6.f - 3.f); }
        CONSTRAIN(params.inversion, -2, 2)
        float maxA, maxD, maxR;
        // if (pp_eg_slow_fast) {
        //     maxA = 60.f;
        //     maxD = 40.f;
        //     maxR = 40.f;
        // } else {
        maxA = 2.f;
        maxD = 2.f;
        maxR = 10.f;
        // }
        params.attack = static_cast<float>(pp_attack) * maxA / 4095.f;
        // if (cv_pp_attack != -1) { params.attack = fabsf(data.cv[cv_pp_attack]) * maxA; }
        CONSTRAIN(params.attack, 0.f, maxA)
        params.decay = static_cast<float>(pp_decay) * maxD / 4095.f;
        // if (cv_pp_decay != -1) { params.decay = fabsf(data.cv[cv_pp_decay]) * maxD; }
        CONSTRAIN(params.decay, 0.f, maxD)
        params.sustain = static_cast<float>(pp_sustain) / 4095.f;
        // if (cv_pp_sustain != -1) { params.sustain = fabsf(data.cv[cv_pp_sustain]); }
        CONSTRAIN(params.sustain, 0.f, 1.f)
        params.release = static_cast<float>(pp_release) * maxR / 4095.f;
        // if (cv_pp_release != -1) { params.release = fabsf(data.cv[cv_pp_release]) * maxR; }
        CONSTRAIN(params.release, 0.f, maxR)

        // vibrato
        params.lfo1_freq = static_cast<float>(pp_lfo1_freq) / 4095.f * 5.f;
        // if (cv_pp_lfo1_freq != -1) { params.lfo1_freq = fabsf(data.cv[cv_pp_lfo1_freq]) * 5.f; }
        CONSTRAIN(params.lfo1_freq, 0.f, 5.f)
        params.lfo1_amt = static_cast<float>(pp_lfo1_amt) / 4095.f * 5.f;
        // if (cv_pp_lfo1_amt != -1) { params.lfo1_amt = fabsf(data.cv[cv_pp_lfo1_amt]) * 5.f; }
        CONSTRAIN(params.lfo1_amt, 0.f, 5.f)

        // filter fm chopper
        params.lfo2_freq = static_cast<float>(pp_lfo2_freq) / 4095.f * 5.f;
        // if (cv_pp_lfo2_freq != -1) { params.lfo2_freq = fabsf(data.cv[cv_pp_lfo2_freq]) * 5.f; }
        CONSTRAIN(params.lfo2_freq, 0.f, 5.f)

        params.lfo2_amt = static_cast<float>(pp_lfo2_amt) / 4095.f;
        // if (cv_pp_lfo2_amt != -1) { params.lfo2_amt = fabsf(data.cv[cv_pp_lfo2_amt]); }
        CONSTRAIN(params.lfo2_amt, 0.f, 1.f)

        params.lfo2_random_phase = pp_lfo2_rphase;

        params.eg_filt_amt = static_cast<float>(pp_eg_filt_amt) / 4095.f;
        // if (cv_pp_eg_filt_amt != -1) { params.eg_filt_amt = data.cv[cv_pp_eg_filt_amt]; }
        CONSTRAIN(params.eg_filt_amt, -1.f, 1.f)

        params.filter_type = pp_filter_type * 2 / 4096;
        // if (cv_pp_filter_type != -1) { params.filter_type = fabsf(data.cv[cv_pp_filter_type]) * 3.f; }
        CONSTRAIN(params.filter_type, 0, 2)        // printf("PP2 %d %d %d\n", paarms.pitch, params.chord, params.nnotes);

        // find a silent voice and activate
        for(int i=0;i<NCVoices;i++){
            // find a dead voice
            if(pp_v_voices[i].IsDead()){
                pp_v_voices[i].Init(params);
                break;
            }
            // if none found, activate the last one
            if(i == NCVoices-1){
                pp_v_voices[i].Init(params);
            }
        }

        pp_latchVoice = true;
    }

    // render buffers with updated cutoff, resonance and detune
    uint32_t c = pp_cutoff * 4;
    // if (cv_pp_cutoff != -1) {
    //     c = static_cast<int32_t>(1750.f + fabsf(data.cv[cv_pp_cutoff]) * (16384.f - 1750.f));
    CONSTRAIN(c, 1750, 16384)
    // }
    int32_t r = pp_resonance * 8;
    // if (cv_pp_resonance != -1) {
    //     r = static_cast<int32_t>(fabsf(data.cv[cv_pp_resonance]) * 32767.f);
    CONSTRAIN(r, 0, 32767)
    // }
    int32_t d = pp_detune * 8;
    // if (cv_pp_detune != -1) {
    //     d = static_cast<int32_t>(fabsf(data.cv[cv_pp_detune]) * 32767.f);
    CONSTRAIN(d, 0, 32767)
    // }

    for (int i=0;i<NCVoices;i++) {
        if(pp_v_voices[i].IsDead()) continue;
        pp_v_voices[i].SetCutoff(c);
        pp_v_voices[i].SetResonance(r);
        pp_v_voices[i].SetDetune(d);
        pp_v_voices[i].Process(pp_out_stereo, 0);
    }

    // it isn't stereo after all...
    for(int i=0;i<BUF_SZ;i++) {
        pp_out_stereo[i*2+0] = pp_out_stereo[i*2+0] * 2.0;
        pp_out_stereo[i*2+1] = pp_out_stereo[i*2+0];
    }

    // note off including latched mode
    // bool shouldNoteOff = true; // !pp_enableEG;
    // if (trig_pp_enableEG != -1) shouldNoteOff = data.trig[trig_pp_enableEG]; // already inverted
    // if (pp_latchEG) {
    //     shouldNoteOff = !shouldNoteOff;
    //     if (!pp_latched && shouldNoteOff)
    //         shouldNoteOff = false;
    // }
    // shouldNoteOff = shouldNoteOff && (pp_latchVoice == true);

    if (pp_latchVoice) {
        for (auto &v:pp_v_voices) {
            v.NoteOff();
        }
        pp_latchVoice = false;
    }

    if (pp_out_stereo[0] != pp_out_stereo[0]) {
        printf("RackPolyPad: NaN detected!\n");
        // hh2.Init();
        // for (auto &v:pp_v_voices) {
        //     v.NoteOff();
        // }
    }
}
