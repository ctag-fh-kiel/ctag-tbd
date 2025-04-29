/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <atomic>
#include <tbd/sound_processor.hpp>
#include "synthesis/RomplerVoice.hpp"
#include <tbd/sound_utils/ctagSampleRom.hpp>
#include <memory>
#include <vector>

using namespace CTAG::SYNTHESIS;

namespace tbd::sounds {

struct SoundProcessorRompler : audio::SoundProcessor {

	virtual void Process(const audio::ProcessData&) override;
	virtual void Init(std::size_t blockSize, void *blockPtr) override;
	virtual ~SoundProcessorRompler();

protected:

	[[tbd(name="Gain", abs, norm=4095, scale=2)]] 
	ufloat_par gain;

	[[tbd(abs, scale=16, min=0, max=14)]] 
	uint_par brr;
 
	trigger_par gate;
	trigger_par latch;

	[[tbd(sft, scale=32, min=0, max=31)]] 
	uint_par bank;

	[[tbd(sft, scale=32, min=0, max=31)]]
	uint_par slice;

	trigger_par slontrg;
	trigger_par skpwt;

	[[tbd(abs, sft, norm=2048, scale=2)]]
	ufloat_par speed;

	[[tbd(scale=60)]]
	float_par pitch;

	[[tbd(sft, norm=2048, scale=12)]]
	ufloat_par tune;

	[[tbd(abs, norm=1048576)]]
	ufloat_par start;

	[[tbd(abs, norm=1048576)]]
	ufloat_par length;

	trigger_par duo;
	trigger_par loop;
	trigger_par looppipo;

	[[tbd(norm=1048576)]]
	ufloat_par lpstart;
	 
	[[tbd(scale=4, min=0, max=3)]]
	uint_par fmode;

	[[tbd(norm=4095)]]
	ufloat_par fcut;

	[[tbd(norm=4095, scale=20)]]
	ufloat_par freso;

	[[tbd(norm=4095)]]
	ufloat_par lfo2am;

	[[tbd(norm=4095, scale=20)]]
	ufloat_par lfo2fm;

	[[tbd(norm=4095)]]
	ufloat_par lfo2filtfm;

	[[tbd(sft, norm=4095)]]
	ufloat_par eg2am;

	[[tbd(norm=4095, scale=20)]]
	ufloat_par eg2fm;

	[[tbd(sft, norm=4095, scale=20)]]
	ufloat_par eg2filtfm;

	[[tbd(norm=4095, scale=20)]]
	ufloat_par lfospeed;

	trigger_par egfasl;

	[[tbd(abs, norm=4095, scale=10)]]
	ufloat_par attack;

	[[tbd(abs, norm=4095, scale=10)]]
	ufloat_par decay;

	[[tbd(abs, scale=4095)]]
	ufloat_par sustain;

	[[tbd(abs, norm=4095, scale=10)]]
	ufloat_par release;

	trigger_par egstop;

protected:

	RomplerVoice romplers[2];
	float out[32];
	bool preGate = false;
	bool bGate2 = false;
	bool preGateLatch = false;
	uint32_t activeVoice = 0;
	uint32_t nextVoice = 0;
	uint32_t wtSliceOffset = 0;
	sound_utils::ctagSampleRom sampleRom;
};

}