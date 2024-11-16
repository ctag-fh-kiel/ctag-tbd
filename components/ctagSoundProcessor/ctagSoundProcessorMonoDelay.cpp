#include "ctagSoundProcessorMonoDelay.hpp"
#include "esp_heap_caps.h"
#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include "helpers/ctagFastMath.hpp"
#include "stmlib/dsp/units.h"


using namespace CTAG::SP;

// on delays https://www.kvraudio.com/forum/viewtopic.php?t=265070
// https://www.kvraudio.com/forum/viewtopic.php?t=382266

/* mifx engine version
void ctagSoundProcessorMonoDelay::Process(const ProcessData &data) {
	float fDelayTime = time_ms; if(cv_time_ms != -1) fDelayTime = fabsf(data.cv[cv_time_ms]);
	// convert fDelayTime to taps
	float ofs = fDelayTime * 44.1f;

	MK_FLT_PAR_ABS(fFeedback, feedback, 4095.f, 1.f)
	MK_FLT_PAR_ABS(fTone, tone, 4095.f, 1.f)
	MK_FLT_PAR_ABS(fMix, mix, 4095.f, 1.f)

	typedef E::Reserve<65535> Memory;
	E::DelayLine<Memory, 0> line;
	E::Context c;

	// simple echo effect
	for(int i=0;i<32;i++){
		if(delayOffset != ofs){
			delayOffset = ONE_POLE(delayOffset, ofs, 0.00001f);
		}
		engine.Start(&c);
		const float in = data.buf[i*2];
		float out;
		c.Interpolate(line, delayOffset, .5f);
		c.Write(out);
		c.Load(in);
		c.Read(out, fFeedback);
		c.Write(line,0.f);

		// mix in and out
		data.buf[i*2] = (1.f - fMix) * in + fMix * out;
		data.buf[i*2 + 1] = data.buf[i*2];
	}
}
*/

void ctagSoundProcessorMonoDelay::Process(const ProcessData &data) {
	MK_FLT_PAR_ABS(fFeedback, feedback, 4095.f, 1.05f)
	MK_FLT_PAR_ABS_SFT(fTone, tone, 4095.f, 1.f)
	MK_FLT_PAR_ABS(fMix, mix, 4095.f, 1.f)
	MK_BOOL_PAR(bTapeDigital, tape_digital)
	MK_BOOL_PAR(bFreeze, freeze)
	bool bSync = sync;
	bool bSyncTrig {false};
	if(trig_sync != -1) bSyncTrig = data.trig[trig_sync] == 1 ? false : true;
	if(!bSync){
		fDelayTime = time_ms;
		if(cv_time_ms != -1) fDelayTime = fabsf(data.cv[cv_time_ms]) * 2000.f;
	}


	float toneFilterCutoff;
	if(fTone > 0.f) toneFilterCutoff = stmlib::SemitonesToRatio(fabsf(fTone) * 120.f);
	else toneFilterCutoff = 10000.f - 20.f * stmlib::SemitonesToRatio((fabsf(fTone)) * 120.f);
	CONSTRAIN(toneFilterCutoff, 100.f, 22000.f)
	toneFilter.set_f<stmlib::FREQUENCY_ACCURATE>(toneFilterCutoff / 44100.f);
	//printf("toneFilterCutoff %f\n", toneFilterCutoff);

	// sync mechanism
	if(bSyncTrig != pre_sync){
		pre_sync = bSyncTrig;
		if(bSyncTrig && bSync){
			int delta = timer - pre_timer;
			if(std::abs(delta) > 1){
				fDelayTime = static_cast<float>(timer) * 32.f / 44.1f;
			}
			pre_timer = timer;
			timer = 0;
		}
	}
	timer++;

	// audio data processing
	CONSTRAIN(fDelayTime, 0.0001, 2000.f)
	float ofs = fDelayTime * 44.1f;
	if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
	for(int i=0; i<32; i++){
		// Calculate the delay offset in samples
		if(delayOffset != ofs){
			if(bTapeDigital){
				if(ofs != delayOffset){
					duck = 1.f;
				}
				delayOffset = ofs;
			} else {
				delayOffset = ONE_POLE(delayOffset, ofs, 0.0001f);
			}
			readPos = static_cast<float>(writeIndex) - delayOffset;
			if(readPos < 0.f) readPos += 88200.f;
			if(readPos >= 88200.f) readPos -= 88200.f;
		}

		float inputSample = data.buf[i*2 + this->processCh];
		float outputSample;

		MAKE_INTEGRAL_FRACTIONAL(readPos);
		outputSample = HELPERS::InterpolateWaveLinearWrap(delayBuffer, readPos, 88200);
		readPos += 1.f;
		readPos > 88200.f ? readPos -= 88200.f : readPos;

		duck = ONE_POLE(duck, 0.f, 0.35f)
		outputSample = outputSample * (1.f - duck);
		// Write the input sample to the delay buffer
		float out;
		if(!bFreeze){
			if(fTone <= -0.1f)
				out = inputSample + toneFilter.Process<stmlib::FILTER_MODE_LOW_PASS>(outputSample * fFeedback * fFeedback);
			else if(fTone >= 0.1f)
				out = inputSample + toneFilter.Process<stmlib::FILTER_MODE_HIGH_PASS>( outputSample * fFeedback * fFeedback);
			else
				out = inputSample + outputSample * fFeedback * fFeedback;
		}
		else
			out = outputSample;
		delayBuffer[writeIndex] = stmlib::SoftLimit(out);
		writeIndex = (writeIndex + 1) % 88200;

		// Mix the dry (input) and wet (delayed) signal
		data.buf[i*2 + this->processCh] = (1.0f - fMix) * inputSample + fMix * outputSample;
		//data.buf[i*2 + 1] = data.buf[i*2];
	}

}

void ctagSoundProcessorMonoDelay::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM

	delayBuffer = static_cast<float*>(heap_caps_malloc(88200 * sizeof(float), MALLOC_CAP_SPIRAM));
	assert(delayBuffer != nullptr);
	// engine.Init(delayBuffer);

}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorMonoDelay::~ctagSoundProcessorMonoDelay() {
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM

	heap_caps_free(delayBuffer);
	toneFilter.Init();
}

void ctagSoundProcessorMonoDelay::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("time_ms", [&](const int val){ time_ms = val;});
	pMapCv.emplace("time_ms", [&](const int val){ cv_time_ms = val;});
	pMapPar.emplace("sync", [&](const int val){ sync = val;});
	pMapTrig.emplace("sync", [&](const int val){ trig_sync = val;});
	pMapPar.emplace("freeze", [&](const int val){ freeze = val;});
	pMapTrig.emplace("freeze", [&](const int val){ trig_freeze = val;});
	pMapPar.emplace("tape_digital", [&](const int val){ tape_digital = val;});
	pMapTrig.emplace("tape_digital", [&](const int val){ trig_tape_digital = val;});
	pMapPar.emplace("feedback", [&](const int val){ feedback = val;});
	pMapCv.emplace("feedback", [&](const int val){ cv_feedback = val;});
	pMapPar.emplace("tone", [&](const int val){ tone = val;});
	pMapCv.emplace("tone", [&](const int val){ cv_tone = val;});
	pMapPar.emplace("mix", [&](const int val){ mix = val;});
	pMapCv.emplace("mix", [&](const int val){ cv_mix = val;});
	isStereo = false;
	id = "MonoDelay";
	// sectionCpp0
}