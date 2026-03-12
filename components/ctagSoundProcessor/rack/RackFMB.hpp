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
#include "synthesis/FmKick.hpp"

using namespace CTAG::SP;

class RackFMB {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	CTAG::SYNTHESIS::FmKick fmb;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> use_ratio_mode;
	atomic<int16_t> mod_env_sync;
	atomic<int16_t> f_b;
	atomic<int16_t> d_b;
	atomic<int16_t> f_m;
	atomic<int16_t> I;
	atomic<int16_t> d_m;
	atomic<int16_t> b_m;
	atomic<int16_t> A_f;
	atomic<int16_t> d_f;
};
