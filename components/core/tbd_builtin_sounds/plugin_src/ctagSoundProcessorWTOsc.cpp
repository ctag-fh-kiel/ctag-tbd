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

#include <tbd/sounds/SoundProcessorWTOsc.hpp>
#include <tbd/sound_utils/ctagNumUtil.hpp>
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"

using namespace tbd::sounds;
using namespace tbd::sound_utils;

void SoundProcessorWTOsc::Process(const sound_processor::ProcessData&data) {
    // wave select
    currentBank = wavebank;
	if(cv_wave != -1) ONE_POLE(fWave, fabsf(data.cv[cv_wave]), 0.1f)
	else fWave = wave / 4095.f;


    if(lastBank != currentBank){ // this is slow, hence not modulated by CV
        prepareWavetables();
        lastBank = currentBank;
    }

    // gain
    MK_FLT_PAR_ABS(fGain, gain, 4095.f, 2.f)

    // adsr + adsr modulation
    MK_BOOL_PAR(bGate, gate)
    adsr.Gate(bGate);
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
    adsr.SetAttack(fAttack);
    adsr.SetDecay(fDecay);
    adsr.SetSustain(fSustain);
    adsr.SetRelease(fRelease);
    // adsr modulation
    MK_FLT_PAR_ABS_SFT(fEGAM, eg2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGFM, eg2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_SFT(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGWave, eg2wave, 4095.f, 1.f)
    valADSR = adsr.Process();

    // modulation LFO
    MK_FLT_PAR_ABS(fLFOSpeed, lfospeed, 4095.f, 20.f)
    MK_BOOL_PAR(bLFOSync, lfosync)
    bool trigger = preGate != bGate && bGate;
    if(bLFOSync && trigger)
        lfo.SetFrequencyPhase(fLFOSpeed, 0.f);
    else
        lfo.SetFrequency(fLFOSpeed);

    preGate = bGate;
    MK_FLT_PAR_ABS(fLFOAM, lfo2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fLFOFM, lfo2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fLFOWave, lfo2wave, 4095.f, 1.f)
    valLFO = lfo.Process();

    // pitch / tuning / FM
    int32_t ipitch = pitch;
    ipitch += 48; // midi note * resolution
    ipitch *= 128;
    int32_t ipitch_root = ipitch;
    if (cv_pitch != -1) {
        ipitch += static_cast<int32_t>(data.cv[cv_pitch] * 12.f * 5.f * 128.f); // five octaves
    }
    int32_t sc = q_scale;
    if (cv_q_scale != -1) {
        sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
        CONSTRAIN(sc, 0, 47);
    }
    //TBD_LOGE("WTOSC", "Scale %d", sc);
    pitchQuantizer.Configure(braids::scales[sc]);
    ipitch = pitchQuantizer.Process(ipitch, ipitch_root);

    float fPitch = static_cast<float>(ipitch);
    fPitch /= 128.f;
    MK_FLT_PAR_ABS_SFT(fTune, tune, 2048.f, 1.f)
    const float f0 = plaits::NoteToFrequency(fPitch + fTune * 12.f + fLFOFM * valLFO + fEGFM * valADSR) * 0.998f;

    // filter
    MK_FLT_PAR_ABS(fCut, fcut, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fReso, freso, 4095.f, 20.f)
    // filter modulation
    fCut = fCut + fEGFMFilt * valADSR + fLFOFMFilt * valLFO; // TODO: Pitch tracking
    // limit values
    CONSTRAIN(fCut, 0.f, 1.f)
    CONSTRAIN(fReso, 1.f, 20.f)
    fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
    svf.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
    MK_INT_PAR_ABS(iFType, fmode, 4.f)
    CONSTRAIN(iFType, 0, 3);

    // calculate modulation params
    float fAM = valADSR * fEGAM; // adsr
    if (fEGAM < 0.f) fAM -= fEGAM; // adsr
    fAM = ((1.f - fabsf(fEGAM)) + fAM); // adsr
    fAM *= (1.f - (valLFO + 1.f) * 0.5f * fLFOAM); // lfo
    fAM *= fGain * fGain; // gain (quadratic)
    CONSTRAIN(fAM, 0.f, 1.f)

    float fWt = fWave + valADSR * fEGWave + valLFO * fLFOWave * 2.f;
    CONSTRAIN(fWt, 0.f, 1.f)

    // detect very fast modulations and filter wave for respective frame
    float deltaWt = fabsf(pre_fWt - fWt);
    if(deltaWt > 0.1f){
        trigger = true;
    }
    pre_fWt = fWt;

    // calc wave and apply filter
    float out[32] = {0.f};
    if(isWaveTableGood){
        oscillator.Render(trigger, f0, fAM, fWt, wavetables, out, bufSz);

        switch(iFType){
            case 1:
                svf.Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, bufSz);
                break;
            case 2:
                svf.Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, bufSz);
                break;
            case 3:
                svf.Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, bufSz);
            default:
                break;
        }
    }

    // convert buffer with interleaving
    for(int i=0;i<bufSz;i++){
        data.buf[i*2 + processCh] = out[i];
    }
}

void SoundProcessorWTOsc::Init(std::size_t blockSize, void *blockPtr) {
    lfo.SetSampleRate( 44100.f / bufSz);
    lfo.SetFrequency(1.f);
    // alloc mem for one wavetable
    // 260 = wavetable size after prep, 64 wavetables, 2 bytes per sample (int16)
    int totalBlockSzRequired = 260*64*2 + 512*4;
    assert(totalBlockSzRequired < blockSize);
    buffer = (int16_t*)blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 260 * 64 * 2;
    memset(buffer, 0, 260 * 64 * 2);
    fbuffer = (float*)blockPtr;
    memset(fbuffer, 0, 512 * 4);


    oscillator.Init();
    svf.Init();
    adsr.SetModeExp();
    adsr.SetSampleRate(44100.f / bufSz);
    adsr.Reset();
    pitchQuantizer.Init();
}

SoundProcessorWTOsc::~SoundProcessorWTOsc() {
}
