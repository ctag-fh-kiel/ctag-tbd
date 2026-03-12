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
#include "RackChannelMixer.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

#define minVolume 0.000001f
#define maxFXSendLevelDly 2.f
#define maxFXSendLevelRev 1.5f

void RackChannelMixer::Init(const PickSeqRackInitData *initdata) {
	cc_base = initdata->cc_base;

	// TODO: "Device" was removed, this is off by one.
	initdata->rack->registerParamAndCC(initdata, "lev", 1, [&](const int val){ mix_lev = val;});
	initdata->rack->registerParamAndCC(initdata, "pan", 2, [&](const int val){ mix_pan = val;});
	initdata->rack->registerParamAndCC(initdata, "fx1", 3, [&](const int val){ mix_fx1 = val;});
	initdata->rack->registerParamAndCC(initdata, "fx2", 4, [&](const int val){ mix_fx2 = val;});
	initdata->rack->registerParamAndCC(initdata, "tracklength", 5, [&](const int val){ mix_track_length = val; });

	this->enabled = false;
	this->track_length = 16;
}

void RackChannelMixer::PreProcess(const PicoSeqRackProcessData &data) {
    MK_FLT_PAR_ABS_NOCV(fPan, mix_pan, 4096.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fLev, mix_lev, 4096.f, 2.f); fLev *= fLev;
    MK_FLT_PAR_ABS_NOCV(fFX1Send, mix_fx1, 4095.f, maxFXSendLevelDly); fFX1Send *= fFX1Send;
    MK_FLT_PAR_ABS_NOCV(fFX2Send, mix_fx2, 4095.f, maxFXSendLevelRev); fFX2Send *= fFX2Send;
    MK_FLT_PAR_ABS_NOCV(fTrackLength, mix_track_length, 4096.f, 128.f);

	if (fLev != this->level) {
		// ESP_LOGI("RackChannelMixer", "Level changed from %f to %f", this->level, fLev);
		this->level = fLev;
	}

    this->enabled = level > minVolume;

	fPan = fPan * 2.0f - 1.0f;
	if (fPan != this->pan) {
		// ESP_LOGI("RackChannelMixer", "Pan changed from %f to %f", this->pan, fPan);
		this->pan = fPan;
	}

	fTrackLength = (int)floor(fTrackLength);
	if (fTrackLength != this->track_length) {
		// ESP_LOGI("RackChannelMixer", "Track length changed from %d to %d",
			// this->track_length, (int)fTrackLength);
		this->track_length = (int)fTrackLength;
	}

	this->send1 = fFX1Send;
	this->send2 = fFX2Send;
}
