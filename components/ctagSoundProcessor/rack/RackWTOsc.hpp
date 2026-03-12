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
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "plaits/dsp/oscillator/wavetable_oscillator.h"

using namespace CTAG::SP;

class RackWTOsc {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float out[BUF_SZ];
	void noteOn(uint8_t note, uint8_t vel);
	void noteOff(uint8_t note, uint8_t vel);

private:
	void prepareWavetables(HELPERS::ctagSampleRom *samplerom);
	plaits::WavetableOscillator<256, 64> oscillator;
	ctagSineSource lfo;
	ctagADSREnv adsr;
	stmlib::Svf svf;
	int16_t *buffer = NULL;
	float *fbuffer = NULL;
	float fWave = 0.f;
	const int16_t *wavetables[64];
	int currentBank = 0;
	int lastBank = -1;
	bool isWaveTableGood = false;
	float valADSR = 0.f, valLFO = 0.f;
	bool preGate = false;
	braids::Quantizer pitchQuantizer;
	float pre_fWt = 0.f;
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	atomic<int32_t> gain;
	atomic<int32_t> pitch;
	atomic<int32_t> q_scale;
	atomic<int32_t> tune;
	atomic<int32_t> wavebank;
	atomic<int32_t> wave;
	atomic<int32_t> fmode;
	atomic<int32_t> fcut;
	atomic<int32_t> freso;
	atomic<int32_t> lfo2wave;
	atomic<int32_t> lfo2am;
	atomic<int32_t> lfo2fm;
	atomic<int32_t> lfo2filtfm;
	atomic<int32_t> eg2wave;
	atomic<int32_t> eg2am;
	atomic<int32_t> eg2fm;
	atomic<int32_t> eg2filtfm;
	atomic<int32_t> lfospeed;
	atomic<int32_t> lfosync;
	atomic<int32_t> egfasl;
	atomic<int32_t> attack;
	atomic<int32_t> decay;
	atomic<int32_t> sustain;
	atomic<int32_t> release;
};
