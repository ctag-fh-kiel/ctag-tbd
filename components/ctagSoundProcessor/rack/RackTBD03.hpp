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
#include "stmlib/dsp/filter.h"
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "filters/ctagDiodeLadderFilter.hpp"
#include "filters/ctagDiodeLadderFilter2.hpp"
#include "filters/ctagDiodeLadderFilter3.hpp"
#include "filters/ctagDiodeLadderFilter4.hpp"
#include "filters/ctagDiodeLadderFilter5.hpp"
#include "filters/ctagFilterBase.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"

using namespace CTAG::SP;

class RackTBD03 {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float td3_out[32];
	void noteOn(uint8_t note, uint8_t vel);
	void noteOff(uint8_t note, uint8_t vel);

private:
    ctagDiodeLadderFilter5 td3_pirkle_zdf_boost; // Pirkle ZDF with boost
    ctagDiodeLadderFilter3 td3_karlson; // Karlson
    ctagDiodeLadderFilter4 td3_blaukraut; // Blaukraut
    ctagDiodeLadderFilter td3_pirkle_zdf; // Pirkle ZDF
    ctagDiodeLadderFilter2 td3_zavalishin; // Zavalishin ZDF
    ctagADEnv td3_adVCA, td3_adVCF;
    braids::MacroOscillator td3_osc;
    braids::SignatureWaveshaper td3_ws;
    uint8_t td3_sync[32] = {0};
    bool td3_pre_trig = false;
    bool td3_isAccent = false;
    float td3_pre_eg_val = 0.f;
    float td3_pre_pitch_val = 0.f;
	int midi_note {0};
	float midi_freq {0.0f};
	bool midi_trig {false};

	atomic<int16_t> td3_sync_trig;
	atomic<int16_t> td3_shape;
	atomic<int16_t> td3_param_0;
	atomic<int16_t> td3_param_1;
	atomic<int16_t> td3_filter_type;
	atomic<int16_t> td3_cutoff;
	atomic<int16_t> td3_resonance;
	atomic<int16_t> td3_envelope;
	atomic<int16_t> td3_saturation;
	atomic<int16_t> td3_drive;
	atomic<int16_t> td3_accent;
	atomic<int16_t> td3_accent_level;
	atomic<int16_t> td3_slide;
	atomic<int16_t> td3_slide_level;
	atomic<int16_t> td3_decay_vca;
	atomic<int16_t> td3_decay_vcf;
	atomic<int16_t> td3_p0_amt;
	atomic<int16_t> td3_p1_amt;
};
