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
	// Single-bank preprocessing: ~33 KB PSRAM for one bank's preprocessed
	// wavetables + a 2 KB float scratch. Bank change in Process triggers
	// a one-shot ~10 ms preprocessing pass (audible block-level glitch on
	// the bank-change tick — acceptable since bank changes are rare and
	// non-real-time). The previous Option-A (preprocess all 32 banks at
	// Init = 1 MiB PSRAM) was reverted because it pushed total free PSRAM
	// below 1 MB and starved the file-manager REST API's rapidjson
	// allocator → Store-access-fault crash on file-manager open.
	static constexpr int kMaxBanks = 32;
	static constexpr int kBankSamples = 260 * 64;          // int16 per bank
	static constexpr int kBankBytes = kBankSamples * 2;

	void prepareBank(int bankIndex, HELPERS::ctagSampleRom *sampleRom);

	int16_t *bankBuffer = nullptr;          // [kBankSamples] — current bank
	float *fbufScratch = nullptr;           // [512] — preprocessing scratch
	plaits::WavetableOscillator<256, 64> oscillator;
	ctagSineSource lfo;
	ctagADSREnv adsr;
	stmlib::Svf svf;
	const int16_t *wavetables[64];
	int currentBank = 0;
	int lastBank = -1;
	bool isWaveTableGood = false;
	float fWave = 0.f;
	float valADSR = 0.f, valLFO = 0.f;
	bool preGate = false;
	braids::Quantizer pitchQuantizer;
	float pre_fWt = 0.f;
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	// Note lifecycle: noteOn → note_held=true + pending_retrigger=true.
	// noteOff → note_held=false. ADSR.Gate is driven by note_held; the
	// retrigger flag forces a fresh attack each noteOn (works around
	// ctagADSREnv::Gate(true) being a no-op in env_decay/env_sustain).
	bool note_held {false};
	uint8_t pending_velocity {100};
	volatile bool pending_retrigger {false};

	// Stuck-note watchdog (mirrors RackRompler's pattern at line 273-296).
	// Reality: the Pico sequencer's stop-path does NOT dispatch noteOff
	// for currently-held notes — held voices on pitched instruments
	// (wtosc, etc.) hang at sustain level forever, requiring a power-
	// cycle. The wrapper-local fix: count blocks since last fresh
	// retrigger; if no new noteOn arrives within kStaleTriggerTicks
	// blocks AND note_held is still true, assume a dropped/missing
	// noteOff and force-release (clear note_held → ADSR enters release).
	// 8192 blocks @ 1378 Hz block rate ≈ 5.9 s — longer than any held
	// step duration in a reasonable sequencer pattern (<2 s at 30 BPM
	// 8th notes), short enough that sequencer-stop noise dies within
	// ~6 seconds instead of "until power cycle". Trade-off: a single
	// keyboard note held longer than ~6 s also auto-releases. Acceptable
	// for sequencer-first use.
	static constexpr uint32_t kStaleTriggerTicks = 8192;
	uint32_t trig_age_ticks {0};

	// Silence gate — once the ADSR has fully released and no note is
	// held, emit zeros and skip the engine entirely.
	static constexpr int kSilenceTailBlocks = 6890;   // ~5 s
	int silence_tail_blocks {0};

	// All atomics explicitly zero-initialized. std::atomic<int32_t>'s
	// default constructor leaves the value indeterminate — and a
	// not-yet-pushed parameter being read at boot would put bizarre
	// values on the audio thread. Bipolar params land at "0" (cv mid)
	// once the preset push happens; for the first few audio blocks
	// before that push, 0 is a safer default than garbage.
	atomic<int32_t> gain {0};
	atomic<int32_t> pitch {0};
	atomic<int32_t> q_scale {0};
	atomic<int32_t> tune {2048};       // bipolar mid (no detune)
	atomic<int32_t> wavebank {0};
	atomic<int32_t> wave {0};
	atomic<int32_t> fmode {0};
	atomic<int32_t> fcut {4095};       // filter open
	atomic<int32_t> freso {0};
	atomic<int32_t> lfo2wave {0};
	atomic<int32_t> lfo2am {0};
	atomic<int32_t> lfo2fm {0};
	atomic<int32_t> lfo2filtfm {0};
	atomic<int32_t> eg2wave {2048};    // bipolar mid (no EG mod)
	atomic<int32_t> eg2am {2048};
	atomic<int32_t> eg2fm {2048};
	atomic<int32_t> eg2filtfm {2048};
	atomic<int32_t> lfospeed {0};
	atomic<int32_t> lfosync {0};
	atomic<int32_t> egfasl {0};
	atomic<int32_t> attack {0};
	atomic<int32_t> decay {2048};      // some decay so first note isn't clipped
	atomic<int32_t> sustain {2048};    // some sustain so first note isn't silent
	atomic<int32_t> release {1024};    // moderate release, well below drone-cap
};
