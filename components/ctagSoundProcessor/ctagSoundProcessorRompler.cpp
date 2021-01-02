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

#include "ctagSoundProcessorRompler.hpp"

using namespace CTAG::SP;

// TODO add switch to include wavetables

void ctagSoundProcessorRompler::Process(const ProcessData &data) {

    // duo or monophonic
    MK_BOOL_PAR(bDuo, duo)

    // latch mode
    MK_BOOL_PAR(bLatch, latch)

    // gate with latch mode
    MK_BOOL_PAR(bGate, gate)
    if(!bLatch)
        bGate2 = bGate;
    else{
        if(bGate != preGateLatch && bGate == true)
            bGate2 ^= true;
    }
    preGateLatch = bGate;
    if(bDuo){
        if(bGate2 != preGate && bGate2 == true){
            romplers[nextVoice]->params.gate = bGate2;
            activeVoice = nextVoice;
        }else if(bGate2 != preGate && bGate2 == false){
            romplers[activeVoice]->params.gate = bGate2;
            nextVoice = (nextVoice+1)%2;
        }
        preGate = bGate2;
    }else{
        romplers[activeVoice]->params.gate = bGate2;
        nextVoice = (activeVoice+1)%2;
        romplers[(activeVoice+1)%2]->Reset();
    }

    // slice selection
    MK_BOOL_PAR(bSliceOnTrg, slontrg)
    romplers[activeVoice]->params.sliceLock = bSliceOnTrg;
    MK_INT_PAR_ABS(iBank, bank, 32.f)
    CONSTRAIN(iBank, 0, 31)
    MK_INT_PAR_ABS(iSlice, slice, 32.f)
    CONSTRAIN(iSlice, 0, 31)
    MK_BOOL_PAR(bSkipWt, skpwt)
    iSlice = iBank * 32 + iSlice;
    if(bSkipWt)
        iSlice += wtSliceOffset;
    romplers[activeVoice]->params.slice = iSlice;

    // pitch related items
    MK_FLT_PAR_ABS_SFT(fSpeed, speed, 2048.f, 2.f)
    romplers[activeVoice]->params.playbackSpeed = fSpeed;
    float fPitch = pitch;
    if(cv_pitch != -1){
        fPitch += data.cv[cv_pitch] * 12.f * 5.f;
    }
    romplers[activeVoice]->params.pitch = fPitch;
    MK_FLT_PAR_ABS_SFT(fTune, tune, 2048.f, 12.f)
    romplers[activeVoice]->params.tune = fTune;

    // offsets
    MK_FLT_PAR_ABS(fStart, start, 1048576.f, 1.f)
    romplers[activeVoice]->params.startOffsetRelative = fStart;
    MK_FLT_PAR_ABS(fLength, length, 1048576.f, 1.f)
    romplers[activeVoice]->params.lengthRelative = fLength;

    // gain
    MK_FLT_PAR_ABS(fGain, gain, 4095.f, 2.f)
    romplers[activeVoice]->params.gain = fGain;

    // adsr
    MK_BOOL_PAR(bEGSync, egstop)
    romplers[activeVoice]->params.egSync = bEGSync;
    MK_BOOL_PAR(bEGSlow, egfasl)
    MK_FLT_PAR_ABS(fAttack, attack, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fDecay, decay, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fSustain, sustain, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fRelease, release, 4095.f, 10.f)
    if(bEGSlow){
        fAttack *= 30.f;
        fDecay *= 30.f;
        fRelease *= 30.f;
    }
    romplers[activeVoice]->params.a = fAttack;
    romplers[activeVoice]->params.d = fDecay;
    romplers[activeVoice]->params.s = fSustain;
    romplers[activeVoice]->params.r = fRelease;

    // adsr modulation
    MK_FLT_PAR_ABS_SFT(fEGAM, eg2am, 4095.f, 1.f)
    romplers[activeVoice]->params.egAM = fEGAM;
    MK_FLT_PAR_ABS_SFT(fEGFM, eg2fm, 4095.f, 12.f)
    romplers[activeVoice]->params.egFM = fEGFM;
    MK_FLT_PAR_ABS_SFT(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    romplers[activeVoice]->params.egFMFilter = fEGFMFilt;

    // filter params
    MK_FLT_PAR_ABS(fCut, fcut, 4095.f, 1.f)
    romplers[activeVoice]->params.cutoff = fCut;
    MK_FLT_PAR_ABS(fReso, freso, 4095.f, 20.f)
    romplers[activeVoice]->params.resonance = fReso;
    MK_INT_PAR_ABS(iFType, fmode, 4.f)
    CONSTRAIN(iFType, 0, 3);
    romplers[activeVoice]->params.filterType = static_cast<RomplerVoice::FilterType>(iFType);

    // loop params
    MK_FLT_PAR_ABS(fLoopPos, lpstart, 1048576.f, 1.f)
    romplers[activeVoice]->params.loopMarker = fLoopPos;
    MK_BOOL_PAR(bLoop, loop)
    romplers[activeVoice]->params.loop = bLoop;
    MK_BOOL_PAR(bLoopPipo, looppipo)
    romplers[activeVoice]->params.loopPiPo = bLoopPipo;

    // modulation LFO
    MK_FLT_PAR_ABS(fLFOSpeed, lfospeed, 4095.f, 20.f)
    romplers[activeVoice]->params.lfoSpeed = fLFOSpeed;
    MK_FLT_PAR_ABS(fLFOAM, lfo2am, 4095.f, 1.f)
    romplers[activeVoice]->params.lfoAM = fLFOAM;
    MK_FLT_PAR_ABS(fLFOFM, lfo2fm, 4095.f, 12.f)
    romplers[activeVoice]->params.lfoFM = fLFOFM;
    MK_FLT_PAR_ABS(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f)
    romplers[activeVoice]->params.lfoFMFilter = fLFOFMFilt;

    if(bDuo){
        romplers[0]->Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] = out[i];
        }
        romplers[1]->Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] += out[i];
        }
    }else{
        romplers[activeVoice]->Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] = out[i];
        }
    }

}

ctagSoundProcessorRompler::ctagSoundProcessorRompler() {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    wtSliceOffset = sampleRom.GetFirstNonWaveTableSlice();

    // create voices
    romplers[0] = std::make_unique<RomplerVoice>();
    romplers[1] = std::make_unique<RomplerVoice>();

    // inits
    preGate = false;
    gate = false;
}

ctagSoundProcessorRompler::~ctagSoundProcessorRompler() {
}

void ctagSoundProcessorRompler::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("gain", [&](const int val){ gain = val;});
	pMapCv.emplace("gain", [&](const int val){ cv_gain = val;});
	pMapPar.emplace("gate", [&](const int val){ gate = val;});
	pMapTrig.emplace("gate", [&](const int val){ trig_gate = val;});
	pMapPar.emplace("latch", [&](const int val){ latch = val;});
	pMapTrig.emplace("latch", [&](const int val){ trig_latch = val;});
	pMapPar.emplace("bank", [&](const int val){ bank = val;});
	pMapCv.emplace("bank", [&](const int val){ cv_bank = val;});
	pMapPar.emplace("slice", [&](const int val){ slice = val;});
	pMapCv.emplace("slice", [&](const int val){ cv_slice = val;});
    pMapPar.emplace("slontrg", [&](const int val){ slontrg = val;});
    pMapTrig.emplace("slontrg", [&](const int val){ trig_slontrg = val;});
	pMapPar.emplace("skpwt", [&](const int val){ skpwt = val;});
	pMapTrig.emplace("skpwt", [&](const int val){ trig_skpwt = val;});
	pMapPar.emplace("speed", [&](const int val){ speed = val;});
	pMapCv.emplace("speed", [&](const int val){ cv_speed = val;});
	pMapPar.emplace("pitch", [&](const int val){ pitch = val;});
	pMapCv.emplace("pitch", [&](const int val){ cv_pitch = val;});
	pMapPar.emplace("tune", [&](const int val){ tune = val;});
	pMapCv.emplace("tune", [&](const int val){ cv_tune = val;});
	pMapPar.emplace("start", [&](const int val){ start = val;});
	pMapCv.emplace("start", [&](const int val){ cv_start = val;});
	pMapPar.emplace("length", [&](const int val){ length = val;});
	pMapCv.emplace("length", [&](const int val){ cv_length = val;});
	pMapPar.emplace("duo", [&](const int val){ duo = val;});
	pMapTrig.emplace("duo", [&](const int val){ trig_duo = val;});
	pMapPar.emplace("loop", [&](const int val){ loop = val;});
	pMapTrig.emplace("loop", [&](const int val){ trig_loop = val;});
	pMapPar.emplace("looppipo", [&](const int val){ looppipo = val;});
	pMapTrig.emplace("looppipo", [&](const int val){ trig_looppipo = val;});
	pMapPar.emplace("lpstart", [&](const int val){ lpstart = val;});
	pMapCv.emplace("lpstart", [&](const int val){ cv_lpstart = val;});
	pMapPar.emplace("fmode", [&](const int val){ fmode = val;});
	pMapCv.emplace("fmode", [&](const int val){ cv_fmode = val;});
	pMapPar.emplace("fcut", [&](const int val){ fcut = val;});
	pMapCv.emplace("fcut", [&](const int val){ cv_fcut = val;});
	pMapPar.emplace("freso", [&](const int val){ freso = val;});
	pMapCv.emplace("freso", [&](const int val){ cv_freso = val;});
	pMapPar.emplace("lfo2am", [&](const int val){ lfo2am = val;});
	pMapCv.emplace("lfo2am", [&](const int val){ cv_lfo2am = val;});
	pMapPar.emplace("lfo2fm", [&](const int val){ lfo2fm = val;});
	pMapCv.emplace("lfo2fm", [&](const int val){ cv_lfo2fm = val;});
	pMapPar.emplace("lfo2filtfm", [&](const int val){ lfo2filtfm = val;});
	pMapCv.emplace("lfo2filtfm", [&](const int val){ cv_lfo2filtfm = val;});
	pMapPar.emplace("eg2am", [&](const int val){ eg2am = val;});
	pMapCv.emplace("eg2am", [&](const int val){ cv_eg2am = val;});
	pMapPar.emplace("eg2fm", [&](const int val){ eg2fm = val;});
	pMapCv.emplace("eg2fm", [&](const int val){ cv_eg2fm = val;});
	pMapPar.emplace("eg2filtfm", [&](const int val){ eg2filtfm = val;});
	pMapCv.emplace("eg2filtfm", [&](const int val){ cv_eg2filtfm = val;});
	pMapPar.emplace("lfospeed", [&](const int val){ lfospeed = val;});
	pMapCv.emplace("lfospeed", [&](const int val){ cv_lfospeed = val;});
	pMapPar.emplace("egfasl", [&](const int val){ egfasl = val;});
	pMapTrig.emplace("egfasl", [&](const int val){ trig_egfasl = val;});
	pMapPar.emplace("attack", [&](const int val){ attack = val;});
	pMapCv.emplace("attack", [&](const int val){ cv_attack = val;});
	pMapPar.emplace("decay", [&](const int val){ decay = val;});
	pMapCv.emplace("decay", [&](const int val){ cv_decay = val;});
	pMapPar.emplace("sustain", [&](const int val){ sustain = val;});
	pMapCv.emplace("sustain", [&](const int val){ cv_sustain = val;});
	pMapPar.emplace("release", [&](const int val){ release = val;});
	pMapCv.emplace("release", [&](const int val){ cv_release = val;});
	pMapPar.emplace("egstop", [&](const int val){ egstop = val;});
	pMapTrig.emplace("egstop", [&](const int val){ trig_egstop = val;});
	isStereo = false;
	id = "Rompler";
	// sectionCpp0
}
