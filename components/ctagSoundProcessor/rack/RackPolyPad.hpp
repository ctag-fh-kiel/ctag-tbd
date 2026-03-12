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

#pragma once
#include "RackSynth.hpp"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "polypad/ChordSynth.hpp"

using namespace CTAG::SP;

class RackPolyPad {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float pp_out_stereo[BUF_SZ * 2];
	void noteOn(uint8_t note, uint8_t vel);
	void noteOff(uint8_t note, uint8_t vel);

private:
    bool pp_trig_prev {false};
	array<ChordSynth, 1> pp_v_voices;
	bool pp_latchVoice = false;
	bool pp_latched = false;
	bool pp_toggle = false;
	int32_t pp_preNCVoices = 0;
	braids::Quantizer pp_quantizer;
	bool trig_prev {false};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	atomic<int16_t> pp_q_scale;
	atomic<int16_t> pp_chord;
	atomic<int16_t> pp_inversion;
	atomic<int16_t> pp_detune;
	atomic<int16_t> pp_nnotes;
	atomic<int16_t> pp_voicehold;
	atomic<int16_t> pp_lfo1_freq;
	atomic<int16_t> pp_lfo1_amt;
	atomic<int16_t> pp_filter_type;
	atomic<int16_t> pp_cutoff;
	atomic<int16_t> pp_resonance;
	atomic<int16_t> pp_lfo2_freq;
	atomic<int16_t> pp_lfo2_amt;
	atomic<int16_t> pp_lfo2_rphase;
	atomic<int16_t> pp_eg_filt_amt;
	atomic<int16_t> pp_attack;
	atomic<int16_t> pp_decay;
	atomic<int16_t> pp_sustain;
	atomic<int16_t> pp_release;
};
