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

using namespace CTAG::SP;

class RackChannelMixer {
public:
	void PreProcess(const PicoSeqRackProcessData &data);
	void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float level;
	float pan;
	float send1;
	float send2;
	int cc_base;
	int track_length;

private:
	atomic<int16_t> mix_lev;
	atomic<int16_t> mix_device;
	atomic<int16_t> mix_pan;
	atomic<int16_t> mix_fx1;
	atomic<int16_t> mix_fx2;
	atomic<int16_t> mix_track_length;
};
