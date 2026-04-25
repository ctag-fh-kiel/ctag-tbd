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
	this->volumeMultiplier = 1.0f;
}

void RackChannelMixer::PreProcess(const PicoSeqRackProcessData &data) {
    // Pan: atomic max is 4064 (CC wire 127 × 32), not 4096. Dividing by
    // 4096 leaves fPan ≈ 0.984 at hard right after the centre-shift, so
    // mL = 1 - 0.984 = 0.016 — 1.6 % bleed on the silenced channel.
    // 4064 + CONSTRAIN gives exact 0/1 at the extremes.
    MK_FLT_PAR_ABS_NOCV(fPan, mix_pan, 4064.f, 1.f)
    CONSTRAIN(fPan, 0.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fLev, mix_lev, 4096.f, 2.f); fLev *= fLev;
    MK_FLT_PAR_ABS_NOCV(fFX1Send, mix_fx1, 4095.f, maxFXSendLevelDly); fFX1Send *= fFX1Send;
    MK_FLT_PAR_ABS_NOCV(fFX2Send, mix_fx2, 4095.f, maxFXSendLevelRev); fFX2Send *= fFX2Send;
    MK_FLT_PAR_ABS_NOCV(fTrackLength, mix_track_length, 4096.f, 128.f);

	fLev *= volumeMultiplier;

	if (fLev != this->level) {
		ESP_LOGI("RackChannelMixer", "Level changed from %f to %f", this->level, fLev);
		this->level = fLev;
	}

    // `muted` is set from the Pico via SoundProcessorManager::SetTrackMute.
    // Gating `enabled` here silences the Input track's continuous audio and
    // cuts synth tails on tracks 1-15 the moment the user toggles mute.
    this->enabled = (level > minVolume) && !muted;

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
