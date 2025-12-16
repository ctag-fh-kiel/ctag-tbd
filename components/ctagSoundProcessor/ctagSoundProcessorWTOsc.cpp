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

#include "ctagSoundProcessorWTOsc.hpp"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

void ctagSoundProcessorWTOsc::Process(const ProcessData &data) {
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
    //ESP_LOGE("WTOSC", "Scale %d", sc);
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

void ctagSoundProcessorWTOsc::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

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

ctagSoundProcessorWTOsc::~ctagSoundProcessorWTOsc() {
}

void ctagSoundProcessorWTOsc::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("gain", [&](const int val){ gain = val;});
	pMapCv.emplace("gain", [&](const int val){ cv_gain = val;});
	pMapPar.emplace("gate", [&](const int val){ gate = val;});
	pMapTrig.emplace("gate", [&](const int val){ trig_gate = val;});
	pMapPar.emplace("pitch", [&](const int val){ pitch = val;});
	pMapCv.emplace("pitch", [&](const int val){ cv_pitch = val;});
	pMapPar.emplace("q_scale", [&](const int val){ q_scale = val;});
	pMapCv.emplace("q_scale", [&](const int val){ cv_q_scale = val;});
	pMapPar.emplace("tune", [&](const int val){ tune = val;});
	pMapCv.emplace("tune", [&](const int val){ cv_tune = val;});
	pMapPar.emplace("wavebank", [&](const int val){ wavebank = val;});
	pMapCv.emplace("wavebank", [&](const int val){ cv_wavebank = val;});
	pMapPar.emplace("wave", [&](const int val){ wave = val;});
	pMapCv.emplace("wave", [&](const int val){ cv_wave = val;});
	pMapPar.emplace("fmode", [&](const int val){ fmode = val;});
	pMapCv.emplace("fmode", [&](const int val){ cv_fmode = val;});
	pMapPar.emplace("fcut", [&](const int val){ fcut = val;});
	pMapCv.emplace("fcut", [&](const int val){ cv_fcut = val;});
	pMapPar.emplace("freso", [&](const int val){ freso = val;});
	pMapCv.emplace("freso", [&](const int val){ cv_freso = val;});
	pMapPar.emplace("lfo2wave", [&](const int val){ lfo2wave = val;});
	pMapCv.emplace("lfo2wave", [&](const int val){ cv_lfo2wave = val;});
	pMapPar.emplace("lfo2am", [&](const int val){ lfo2am = val;});
	pMapCv.emplace("lfo2am", [&](const int val){ cv_lfo2am = val;});
	pMapPar.emplace("lfo2fm", [&](const int val){ lfo2fm = val;});
	pMapCv.emplace("lfo2fm", [&](const int val){ cv_lfo2fm = val;});
	pMapPar.emplace("lfo2filtfm", [&](const int val){ lfo2filtfm = val;});
	pMapCv.emplace("lfo2filtfm", [&](const int val){ cv_lfo2filtfm = val;});
	pMapPar.emplace("eg2wave", [&](const int val){ eg2wave = val;});
	pMapCv.emplace("eg2wave", [&](const int val){ cv_eg2wave = val;});
	pMapPar.emplace("eg2am", [&](const int val){ eg2am = val;});
	pMapCv.emplace("eg2am", [&](const int val){ cv_eg2am = val;});
	pMapPar.emplace("eg2fm", [&](const int val){ eg2fm = val;});
	pMapCv.emplace("eg2fm", [&](const int val){ cv_eg2fm = val;});
	pMapPar.emplace("eg2filtfm", [&](const int val){ eg2filtfm = val;});
	pMapCv.emplace("eg2filtfm", [&](const int val){ cv_eg2filtfm = val;});
	pMapPar.emplace("lfospeed", [&](const int val){ lfospeed = val;});
	pMapCv.emplace("lfospeed", [&](const int val){ cv_lfospeed = val;});
	pMapPar.emplace("lfosync", [&](const int val){ lfosync = val;});
	pMapTrig.emplace("lfosync", [&](const int val){ trig_lfosync = val;});
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
	isStereo = false;
	id = "WTOsc";
	// sectionCpp0
}

void ctagSoundProcessorWTOsc::prepareWavetables() {
    // precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
    // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
    // check if sample rom seems to have current bank
    if(!sample_rom.HasSliceGroup(currentBank * 64, currentBank * 64 + 63)){
        isWaveTableGood = false;
        return;
    }
    int size = sample_rom.GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
    if(size != 256*64){
        isWaveTableGood = false;
        return;
    }
    int bankOffset = currentBank*64;
    int bufferOffset = 4*64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
    sample_rom.ReadSlice(&buffer[bufferOffset], bankOffset, 0, 256*64);
    // start conversion of data
    // 64 wavetables per bank
    int c = 0;
    for(int i=0;i<64;i++){ // iterate all waves
        int startOffset = bufferOffset + i*256; // which wave
        // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
        float sum4 = buffer[startOffset] + buffer[startOffset+1] + buffer[startOffset+2] + buffer[startOffset+3]; // add dc
        for(int j=0;j<512;j++){
            fbuffer[j] = buffer[startOffset + (j%256)] + sum4;
        }
        // x -= x.mean()
        removeMeanOfFloatArray(fbuffer, 512);
        // x /= numpy.abs(x).max()
        scaleFloatArrayToAbsMax(fbuffer, 512);
        // x = numpy.cumsum(x)
        accumulateFloatArray(fbuffer, 512);
        // x -= x.mean()
        removeMeanOfFloatArray(fbuffer, 512);
        // create pointer map
        wavetables[i] = &buffer[c];
        // x = numpy.round(x * (4 * 32768.0 / WAVETABLE_SIZE)
        for(int j=512-256-4;j<512;j++){
            int16_t v = static_cast<int16_t >(roundf(fbuffer[j] * 4.f * 32768.f / 256.f));
            buffer[c++] = v;
        }
    }
    isWaveTableGood = true;
}