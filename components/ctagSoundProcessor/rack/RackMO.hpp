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
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"

using namespace CTAG::SP;

class RackMO {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float mo_out[32];
	void noteOn(uint8_t note, uint8_t vel);
	void noteOff(uint8_t note, uint8_t vel);

private:
	braids::MacroOscillatorShape mo_last_shape;
	braids::MacroOscillator mo_osc;
	braids::SignatureWaveshaper mo_ws;
	braids::Quantizer mo_quantizer;
	CTAG::SP::HELPERS::ctagADEnv mo_envelope;
	const uint8_t mo_sync[32] = {0};
	bool mo_prevTrigger = false;
	const uint16_t mo_bit_reduction_masks[7] = {
			0xc000,
			0xe000,
			0xf000,
			0xf800,
			0xff00,
			0xfff0,
			0xffff};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	atomic<int16_t> mo_shape;
	atomic<int16_t> mo_pitch;
	atomic<int16_t> mo_decimation;
	atomic<int16_t> mo_bit_reduction;
	atomic<int16_t> mo_q_scale;
	atomic<int16_t> mo_param_0;
	atomic<int16_t> mo_param_1;
	atomic<int16_t> mo_waveshaping;
	atomic<int16_t> mo_fm_amt;
	atomic<int16_t> mo_p0_amt;
	atomic<int16_t> mo_p1_amt;
	atomic<int16_t> mo_loopEG;
	atomic<int16_t> mo_attack;
	atomic<int16_t> mo_decay;
};
