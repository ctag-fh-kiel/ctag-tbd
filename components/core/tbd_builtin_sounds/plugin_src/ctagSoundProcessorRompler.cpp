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

#include <tbd/sounds/SoundProcessorRompler.hpp>

using namespace tbd::sounds;

// TODO add switch to include wavetables

void SoundProcessorRompler::Process(const sound_processor::ProcessData&data) {

    // duo or monophonic
    // MK_BOOL_PAR(bDuo, duo)

    // latch mode
    // MK_BOOL_PAR(bLatch, latch)

    // gate with latch mode
    // MK_BOOL_PAR(bGate, gate)
    if(!latch)
        bGate2 = gate;
    else{
        if(gate != preGateLatch && gate)
            bGate2 ^= true;
    }
    preGateLatch = gate;
    if (gate) {
        if (bGate2 != preGate && bGate2 == true) {
            romplers[nextVoice].params.gate = bGate2;
            activeVoice = nextVoice;
        } else if (bGate2 != preGate && bGate2 == false) {
            romplers[activeVoice].params.gate = bGate2;
            nextVoice = (nextVoice+1)%2;
        }
        preGate = bGate2;
    } else {
        romplers[activeVoice].params.gate = bGate2;
        nextVoice = (activeVoice+1)%2;
        romplers[(activeVoice+1)%2].Reset();
    }

    // slice selection
    // MK_BOOL_PAR(bSliceOnTrg, slontrg)
    romplers[activeVoice].params.sliceLock = slontrg;
    // MK_INT_PAR_ABS(iBank, bank, 32.f)
    // CONSTRAIN(iBank, 0, 31)
    // MK_INT_PAR_ABS(iSlice, slice, 32.f)
    // CONSTRAIN(iSlice, 0, 31)
    // MK_BOOL_PAR(bSkipWt, skpwt)
    slice = bank * 32 + slice;
    if(skpwt)
        slice += sampleRom.GetFirstNonWaveTableSlice();
    romplers[activeVoice].params.slice = slice;

    // pitch related items
    // MK_FLT_PAR_ABS_SFT(fSpeed, speed, 2048.f, 2.f)
    romplers[activeVoice].params.playbackSpeed = speed;
    // float fPitch = pitch;
    // if(cv_pitch != -1){
    //     fPitch += data.cv[cv_pitch] * 12.f * 5.f;
    // }
    romplers[activeVoice].params.pitch = pitch;
    // MK_FLT_PAR_ABS_SFT(fTune, tune, 2048.f, 12.f)
    romplers[activeVoice].params.tune = tune;

    // offsets
    // MK_FLT_PAR_ABS(fStart, start, 1048576.f, 1.f)
    romplers[activeVoice].params.startOffsetRelative = start;
    // MK_FLT_PAR_ABS(fLength, length, 1048576.f, 1.f)
    romplers[activeVoice].params.lengthRelative = length;

    // gain
    // MK_FLT_PAR_ABS(fGain, gain, 4095.f, 2.f)
    romplers[activeVoice].params.gain = gain;

    // bit rate reduction
    // MK_INT_PAR_ABS(iBrr, brr, 16)
    // CONSTRAIN(iBrr, 0, 14)
    romplers[activeVoice].params.bitReduction = brr;

    // adsr
    // MK_BOOL_PAR(bEGSync, egstop)
    romplers[activeVoice].params.egSync = egstop;
    // MK_BOOL_PAR(bEGSlow, egfasl)
    // MK_FLT_PAR_ABS(fAttack, attack, 4095.f, 10.f)
    // MK_FLT_PAR_ABS(fDecay, decay, 4095.f, 10.f)
    // MK_FLT_PAR_ABS(fSustain, sustain, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fRelease, release, 4095.f, 10.f)
    if(egfasl){
        attack *= 30.f;
        decay *= 30.f;
        release *= 30.f;
    }
    romplers[activeVoice].params.a = attack;
    romplers[activeVoice].params.d = decay;
    romplers[activeVoice].params.s = sustain;
    romplers[activeVoice].params.r = release;

    // adsr modulation
    // MK_FLT_PAR_ABS_SFT(fEGAM, eg2am, 4095.f, 1.f)
    romplers[activeVoice].params.egAM = eg2am;
    // MK_FLT_PAR_ABS_SFT(fEGFM, eg2fm, 4095.f, 12.f)
    romplers[activeVoice].params.egFM = eg2fm;
    // MK_FLT_PAR_ABS_SFT(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    romplers[activeVoice].params.egFMFilter = eg2filtfm;

    // filter params
    // MK_FLT_PAR_ABS(fCut, fcut, 4095.f, 1.f)
    romplers[activeVoice].params.cutoff = fcut;
    // MK_FLT_PAR_ABS(fReso, freso, 4095.f, 20.f)
    romplers[activeVoice].params.resonance = freso;
    // MK_INT_PAR_ABS(iFType, fmode, 4.f)
    // CONSTRAIN(iFType, 0, 3);
    romplers[activeVoice].params.filterType = static_cast<RomplerVoice::FilterType>(fmode);

    // loop params
    // MK_FLT_PAR_ABS(fLoopPos, lpstart, 1048576.f, 1.f)
    romplers[activeVoice].params.loopMarker = lpstart;
    // MK_BOOL_PAR(bLoop, loop)
    romplers[activeVoice].params.loop = loop;
    // MK_BOOL_PAR(bLoopPipo, looppipo)
    romplers[activeVoice].params.loopPiPo = looppipo;

    // modulation LFO
    // MK_FLT_PAR_ABS(fLFOSpeed, lfospeed, 4095.f, 20.f)
    romplers[activeVoice].params.lfoSpeed = lfospeed;
    // MK_FLT_PAR_ABS(fLFOAM, lfo2am, 4095.f, 1.f)
    romplers[activeVoice].params.lfoAM = lfo2am;
    // MK_FLT_PAR_ABS(fLFOFM, lfo2fm, 4095.f, 12.f)
    romplers[activeVoice].params.lfoFM = lfo2fm;
    // MK_FLT_PAR_ABS(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f)
    romplers[activeVoice].params.lfoFMFilter = lfo2filtfm;

    if(duo){
        romplers[0].Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] = out[i];
        }
        romplers[1].Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] += out[i];
        }
    }else{
        romplers[activeVoice].Process(out, bufSz);
        for(int i=0;i<bufSz;i++){
            data.buf[i*2 + processCh] = out[i];
        }
    }

}

void SoundProcessorRompler::Init(std::size_t blockSize, void *blockPtr) {
    // inits
    preGate = false;
    gate = false;

    for(auto &r: romplers)
        r.Init(44100.f);
}

SoundProcessorRompler::~SoundProcessorRompler() {
}
