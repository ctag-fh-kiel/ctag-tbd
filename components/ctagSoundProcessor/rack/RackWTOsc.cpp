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
#include "RackWTOsc.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void RackWTOsc::Init(const PickSeqRackInitData *initdata) {
    lfo.SetSampleRate(44100.f / BUF_SZ);
    lfo.SetFrequency(1.f);

    buffer = static_cast<int16_t*>(heap_caps_malloc(260 * 64 * 2, MALLOC_CAP_SPIRAM));
    fbuffer = static_cast<float*>(heap_caps_malloc(512 * 4, MALLOC_CAP_SPIRAM));
    memset(buffer, 0, 260 * 64 * 2);
    memset(fbuffer, 0, 512 * 4);

    oscillator.Init();
    svf.Init();
    adsr.SetModeExp();
    adsr.SetSampleRate(44100.f / BUF_SZ);
    adsr.Reset();
    pitchQuantizer.Init();

    this->enabled = false;

    initdata->rack->registerParamAndCC(initdata, "wavebank", 8, [&](const int val){ wavebank = val;});
    initdata->rack->registerParamAndCC(initdata, "wave", 9, [&](const int val){ wave = val;});
    initdata->rack->registerParamAndCC(initdata, "tune", 10, [&](const int val){ tune = val;});

    initdata->rack->registerParamAndCC(initdata, "fmode", 11, [&](const int val){ fmode = val;});
    initdata->rack->registerParamAndCC(initdata, "fcut", 12, [&](const int val){ fcut = val;});
    initdata->rack->registerParamAndCC(initdata, "freso", 13, [&](const int val){ freso = val;});
    initdata->rack->registerParamAndCC(initdata, "q_scale", 14, [&](const int val){ q_scale = val;});

    initdata->rack->registerParamAndCC(initdata, "attack", 15, [&](const int val){ attack = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 16, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "sustain", 17, [&](const int val){ sustain = val;});
    initdata->rack->registerParamAndCC(initdata, "release", 18, [&](const int val){ release = val;});

    initdata->rack->registerParamAndCC(initdata, "eg2wave", 19, [&](const int val){ eg2wave = val;});
    // initdata->rack->registerParamAndCC(initdata, "eg2am", 19, [&](const int val){ eg2am = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2fm", 20, [&](const int val){ eg2fm = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2filtfm", 21, [&](const int val){ eg2filtfm = val;});

    initdata->rack->registerParamAndCC(initdata, "lfospeed", 22, [&](const int val){ lfospeed = val;});
    initdata->rack->registerParamAndCC(initdata, "lfosync", 23, [&](const int val){ lfosync = val;});
    // initdata->rack->registerParamAndCC(initdata, "egfasl", 24, [&](const int val){ egfasl = val;});

    initdata->rack->registerParamAndCC(initdata, "lfo2wave", 25, [&](const int val){ lfo2wave = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2am", 26, [&](const int val){ lfo2am = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2fm", 27, [&](const int val){ lfo2fm = val;});
    initdata->rack->registerParamAndCC(initdata, "lfo2filtfm", 28, [&](const int val){ lfo2filtfm = val;});

    initdata->rack->registerParamAndCC(initdata, "gain", 29, [&](const int val){ gain = val;});
    // initdata->rack->registerParamAndCC(initdata, "pitch", 9, [&](const int val){ pitch = val;});
}

void RackWTOsc::noteOn(uint8_t note, uint8_t vel) {
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
    pitch = note * 128.0f;
}

void RackWTOsc::noteOff(uint8_t note, uint8_t vel) {
    midi_trig = false;
}

void RackWTOsc::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    // wave select
    currentBank = (wavebank * 16) / 4096;
	// if(cv_wave != -1) ONE_POLE(fWave, fabsf(data.cv[cv_wave]), 0.1f) else
    fWave = wave / 4095.f;

    if(lastBank != currentBank) { // this is slow, hence not modulated by CV
        prepareWavetables(data.sampleRom);
        lastBank = currentBank;
    }

    // gain
    MK_FLT_PAR_ABS_NOCV(fGain, gain, 4095.f, 2.f)

    // adsr + adsr modulation
    // MK_BOOL_PAR_NOCV(bGate, gate)
    adsr.Gate(midi_trig);
    // MK_BOOL_PAR_NOCV(bEGSlow, egfasl)
    MK_FLT_PAR_ABS_NOCV(fAttack, attack, 4095.f, 10.f)
    MK_FLT_PAR_ABS_NOCV(fDecay, decay, 4095.f, 10.f)
    MK_FLT_PAR_ABS_NOCV(fSustain, sustain, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fRelease, release, 4095.f, 10.f)
    // if(bEGSlow){
    //     fAttack *= 30.f;
    //     fDecay *= 30.f;
    //     fRelease *= 30.f;
    // }
    adsr.SetAttack(fAttack);
    adsr.SetDecay(fDecay);
    adsr.SetSustain(fSustain);
    adsr.SetRelease(fRelease);

    // adsr modulation
    // MK_FLT_PAR_ABS_SFT_NOCV(fEGAM, eg2am, 4095.f, 1.f)
    float fEGAM = 1.0f;
    MK_FLT_PAR_ABS_SFT_NOCV(fEGFM, eg2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_SFT_NOCV(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT_NOCV(fEGWave, eg2wave, 4095.f, 1.f)
    valADSR = adsr.Process();

    // modulation LFO
    MK_FLT_PAR_ABS_NOCV(fLFOSpeed, lfospeed, 4095.f, 20.f)
    MK_BOOL_PAR_NOCV(bLFOSync, lfosync)

    bool trigger = preGate != midi_trig && midi_trig;

    // if (midi_trig) {
    //     // trigger = true;
    //     midi_trig = false;
    // }

    if (trigger) {
        // printf("WTOSC\n");
        if (bLFOSync) {
            lfo.SetFrequencyPhase(fLFOSpeed, 0.f);
        } else {
            lfo.SetFrequency(fLFOSpeed);
        }
    }

    preGate = midi_trig;

    MK_FLT_PAR_ABS_NOCV(fLFOAM, lfo2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fLFOFM, lfo2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_NOCV(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f);
    MK_FLT_PAR_ABS_NOCV(fLFOWave, lfo2wave, 4095.f, 1.f)
    valLFO = lfo.Process();

    // // pitch / tuning / FM
    int32_t ipitch = 0;
    // ipitch += 48; // midi note * resolution
    // ipitch *= 128;
    ipitch += static_cast<int32_t>(midi_note * 128.0f);
    int32_t ipitch_root = ipitch;
    // if (cv_pitch != -1) {
    //     ipitch += static_cast<int32_t>(data.cv[cv_pitch] * 12.f * 5.f * 128.f); // five octaves
    // }
    int32_t sc = q_scale * 48 / 4096;
    // if (cv_q_scale != -1) {
    //     sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
    CONSTRAIN(sc, 0, 47);
    // }
    //ESP_LOGE("WTOSC", "Scale %d", sc);
    // pitchQuantizer.Configure(braids::scales[sc]);
    // ipitch = pitchQuantizer.Process(ipitch, ipitch_root);

    float fPitch = static_cast<float>(ipitch);
    fPitch /= 128.f;
    MK_FLT_PAR_ABS_SFT_NOCV(fTune, tune, 2048.f, 1.f)
    const float f0 = plaits::NoteToFrequency(fPitch + fTune * 12.f + fLFOFM * valLFO + fEGFM * valADSR) * 0.998f;

    // filter
    MK_FLT_PAR_ABS_NOCV(fCut, fcut, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fReso, freso, 4095.f, 20.f)
    // filter modulation
    fCut = fCut + fEGFMFilt * valADSR + fLFOFMFilt * valLFO; // TODO: Pitch tracking
    // limit values
    CONSTRAIN(fCut, 0.f, 1.f)
    CONSTRAIN(fReso, 1.f, 20.f)
    fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
    svf.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
    MK_INT_PAR_ABS_NOCV(iFType, fmode, 4.f)
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
    // float deltaWt = fabsf(pre_fWt - fWt);
    // if(deltaWt > 0.1f){
    //     trigger = true;
    // }
    // pre_fWt = fWt;

    // calc wave and apply filter
    if(isWaveTableGood){
        oscillator.Render(trigger, f0, fAM, fWt, wavetables, out, BUF_SZ);

        switch(iFType){
            case 1:
                svf.Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, BUF_SZ);
                break;
            case 2:
                svf.Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, BUF_SZ);
                break;
            case 3:
                svf.Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, BUF_SZ);
            default:
                break;
        }
    }
}

void RackWTOsc::prepareWavetables(HELPERS::ctagSampleRom *sampleRom) {
    ESP_LOGI("DrumRackWTOsc", "prepareWavetables bank=%d\n", currentBank);

    // precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
    // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
    // check if sample rom seems to have current bank
    if(!sampleRom->HasSliceGroup(currentBank * 64, currentBank * 64 + 63)){
        isWaveTableGood = false;
        return;
    }
    int size = sampleRom->GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
    if(size != 256*64){
        isWaveTableGood = false;
        return;
    }
    int bankOffset = currentBank*64;
    int bufferOffset = 4*64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
    sampleRom->ReadSlice(&buffer[bufferOffset], bankOffset, 0, 256*64);
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
