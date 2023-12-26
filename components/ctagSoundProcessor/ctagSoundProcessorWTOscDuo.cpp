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

#include "ctagSoundProcessorWTOscDuo.hpp"
#include "esp_heap_caps.h"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;
using namespace CTAG::SP::HELPERS;

void ctagSoundProcessorWTOscDuo::Process(const ProcessData &data) {
    // wave select
    currentBank = wavebank;
    MK_FLT_PAR_ABS(fwave_1, wave_1, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fwave_2, wave_2, 4095.f, 1.f)

    if(lastBank != currentBank )
    { // this is slow, hence not modulated by CV
        prepareWavetables();
        lastBank = currentBank;
    }

    // gain
    MK_FLT_PAR_ABS_ADD(fGain_1, gain_1, 4095.f, 2.f)
    MK_FLT_PAR_ABS_ADD(fGain_2, gain_2, 4095.f, 2.f)

    // adsr + adsr modulation
    MK_BOOL_PAR(bGate_1, gate_1)
    adsr_1.Gate(bGate_1);
    MK_BOOL_PAR(bGate_2, gate_2)
    adsr_2.Gate(bGate_2);
    MK_BOOL_PAR(bEGSlow, egfasl)
    MK_FLT_PAR_ABS(fAttack, attack, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fDecay, decay, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fSustain, sustain, 4095.f, 1.f)
    CONSTRAIN(fSustain, 0.f, 1.f)
     MK_FLT_PAR_ABS(fRelease, release, 4095.f, 10.f)
    if(bEGSlow){
        fAttack *= 30.f;
        fDecay *= 30.f;
        fRelease *= 30.f;
    }
    adsr_1.SetAttack(fAttack);
    adsr_1.SetDecay(fDecay);
    adsr_1.SetSustain(fSustain);
    adsr_1.SetRelease(fRelease);

    adsr_2.SetAttack(fAttack);
    adsr_2.SetDecay(fDecay);
    adsr_2.SetSustain(fSustain);
    adsr_2.SetRelease(fRelease);
    // adsr modulation
    MK_FLT_PAR_ABS_SFT(fEGAM, eg2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGFM, eg2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_SFT(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGWave, eg2wave, 4095.f, 1.f)
    valADSR_1 = adsr_1.Process();
    valADSR_2 = adsr_2.Process();

    // modulation LFO
    MK_FLT_PAR_ABS(fLFOSpeed, lfospeed, 4095.f, 20.f)
    MK_FLT_PAR_ABS(fVintageVibe, vintage_vibe, 4095.f, 0.625f)    // Used to slightly detune the LFOs frequencies
    MK_BOOL_PAR(bLFOSync_1, lfosync_1)
    MK_BOOL_PAR(bLFOSync_2, lfosync_2)
    bool trigger1 {preGate_1 != bGate_1 && bGate_1};
    bool trigger2 {preGate_2 != bGate_2 && bGate_2};
    if(bLFOSync_1 && trigger1) // If LFO of voice 1 should be synced: detect EG-trigger of voice 1
        lfo_1.SetFrequencyPhase(fLFOSpeed-fVintageVibe, 0.f);
    else
        lfo_1.SetFrequency(fLFOSpeed-fVintageVibe);  // Always newly set LFO-frequency, it might be changed since last check
    
    if(bLFOSync_2 && trigger2) // If LFO of voice 2 should be synced: detect EG-trigger of voice 1
        lfo_2.SetFrequencyPhase(fLFOSpeed+fVintageVibe, 0.f);
    else
        lfo_2.SetFrequency(fLFOSpeed+fVintageVibe);  // Always newly set LFO-frequency, it might be changed since last check
    
    preGate_1 = bGate_1;
    preGate_2 = bGate_2;
    MK_FLT_PAR_ABS(fLFOAM, lfo2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fLFOFM, lfo2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fLFOWave, lfo2wave, 4095.f, 1.f)
    valLFO_1 = lfo_1.Process();
    valLFO_2 = lfo_2.Process();

    // pitch / tuning / FM
    int32_t ipitch_1 = pitch_1;
    ipitch_1 += 48; // midi note * resolution
    ipitch_1 *= 128;
    int32_t ipitch_1_root = ipitch_1;
    if (cv_pitch_1 != -1) {
        ipitch_1 += static_cast<int32_t>(data.cv[cv_pitch_1] * 12.f * 5.f * 128.f); // five octaves
    }

    // pitch / tuning / FM
    int32_t ipitch_2 = pitch_2;
    ipitch_2 += 48; // midi note * resolution
    ipitch_2 *= 128;
    int32_t ipitch_2_root = ipitch_2;
    if (cv_pitch_2 != -1) {
        ipitch_2 += static_cast<int32_t>(data.cv[cv_pitch_2] * 12.f * 5.f * 128.f); // five octaves
    }

    int32_t sc = q_scale;
    if (cv_q_scale != -1) {
        sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
        CONSTRAIN(sc, 0, 47);
    }
    //ESP_LOGE("WTOscDuo", "Scale %d", sc);
    pitchQuantizer.Configure(braids::scales[sc]);
    ipitch_1 = pitchQuantizer.Process(ipitch_1, ipitch_1_root);
    ipitch_2 = pitchQuantizer.Process(ipitch_2, ipitch_2_root);

    float fpitch_1 = static_cast<float>(ipitch_1);
    fpitch_1 /= 128.f;
    MK_FLT_PAR_ABS_SFT(fTune_1, tune_1, 2048.f, 1.f)
    const float f_1 = plaits::NoteToFrequency(fpitch_1 + fTune_1 * 12.f + fLFOFM * valLFO_1 + fEGFM * valADSR_1) * 0.998f;

    float fpitch_2 = static_cast<float>(ipitch_2);
    fpitch_2 /= 128.f;
    MK_FLT_PAR_ABS_SFT(fTune_2, tune_2, 2048.f, 1.f)
    const float f_2 = plaits::NoteToFrequency(fpitch_2 + fTune_2 * 12.f + fLFOFM * valLFO_2 + fEGFM * valADSR_2) * 0.998f;

    // filter
    MK_FLT_PAR_ABS_ADD(fCut_1, fcut_1, 4095.f, 1.f);      // This is a special variant where the GUI-parameter and the corresponding CV get added
    MK_FLT_PAR_ABS_ADD(fCut_2, fcut_2, 4095.f, 1.f); 
    MK_FLT_PAR_ABS(fReso, freso, 4095.f, 20.f)
    // filter modulation
    fCut_1 = fCut_1 + fEGFMFilt * valADSR_1 + fLFOFMFilt * valLFO_1; // TODO: Pitch tracking
    fCut_2 = fCut_2 + fEGFMFilt * valADSR_2 + fLFOFMFilt * valLFO_2; // TODO: Pitch tracking
    // limit values
    CONSTRAIN(fCut_1, 0.f, 1.f)
    CONSTRAIN(fCut_2, 0.f, 1.f)
    
    CONSTRAIN(fReso, 1.f, 20.f)
    fCut_1 = 20.f * stmlib::SemitonesToRatio(fCut_1 * 120.f);
    svf_1.set_f_q<stmlib::FREQUENCY_FAST>(fCut_1 / 44100.f, fReso);

    fCut_2 = 20.f * stmlib::SemitonesToRatio(fCut_2 * 120.f);
    svf_2.set_f_q<stmlib::FREQUENCY_FAST>(fCut_2 / 44100.f, fReso);

    MK_INT_PAR_ABS(iFType, fmode, 4.f)
    CONSTRAIN(iFType, 0, 3);

    // calculate modulation params
    float fAM_1 = valADSR_1 * fEGAM; // adsr
    if (fEGAM < 0.f) fAM_1 -= fEGAM; // adsr
    fAM_1 = ((1.f - fabsf(fEGAM)) + fAM_1); // adsr
    fAM_1 *= (1.f - (valLFO_1 + 1.f) * 0.5f * fLFOAM); // lfo 
    fAM_1 *= fGain_1 * fGain_1; // gain (quadratic)
    CONSTRAIN(fAM_1, 0.f, 1.f)

    float fAM_2 = valADSR_2 * fEGAM; // adsr
    if (fEGAM < 0.f) fAM_2 -= fEGAM; // adsr
    fAM_2 = ((1.f - fabsf(fEGAM)) + fAM_2); // adsr
    fAM_2 *= (1.f - (valLFO_2 + 1.f) * 0.5f * fLFOAM); // lfo 
    fAM_2 *= fGain_2 * fGain_2; // gain (quadratic)
    CONSTRAIN(fAM_2, 0.f, 1.f)

    float fWt_1 = fwave_1 + valADSR_1 * fEGWave + valLFO_1 * fLFOWave * 2.f;
    CONSTRAIN(fWt_1, 0.f, 1.f)

    float fWt_2 = fwave_2 + valADSR_2 * fEGWave + valLFO_1 * fLFOWave * 2.f;
    CONSTRAIN(fWt_2, 0.f, 1.f)

    // detect very fast modulations and filter wave for respective frame
    float deltaWt1 = fabsf(pre_fWt_1 - fWt_1);
    if(deltaWt1 > 0.1f){
        trigger1 = true;
    }
    pre_fWt_1 = fWt_1;

    float deltaWt2 = fabsf(pre_fWt_2 - fWt_1);
    if(deltaWt2 > 0.1f)
        trigger2 = true;
    pre_fWt_2 = fWt_2;

    // calc wave and apply filter
    float out_1[32] = {0.f};
    if(isWaveTableGood)
    {
        oscillator_1.Render(trigger1, f_1, fAM_1, fWt_1, wavetables, out_1, bufSz);

        switch(iFType){
            case 1:
                svf_1.Process<stmlib::FILTER_MODE_LOW_PASS>(out_1, out_1, bufSz);
                break;
            case 2:
                svf_1.Process<stmlib::FILTER_MODE_BAND_PASS>(out_1, out_1, bufSz);
                break;
            case 3:
                svf_1.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_1, out_1, bufSz);
            default:
                break;
        }
    }
    // calc wave and apply filter
    float out_2[32] = {0.f};
    if(isWaveTableGood)
    {
        oscillator_2.Render(trigger2, f_2, fAM_2, fWt_2, wavetables, out_2, bufSz);

        switch(iFType){
            case 1:
                svf_2.Process<stmlib::FILTER_MODE_LOW_PASS>(out_2, out_2, bufSz);
                break;
            case 2:
                svf_2.Process<stmlib::FILTER_MODE_BAND_PASS>(out_2, out_2, bufSz);
                break;
            case 3:
                svf_2.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_2, out_2, bufSz);
            default:
                break;
        }
    }

    // convert buffer with interleaving
    for(int i=0; i<bufSz; i++)
    {
        data.buf[i*2 + processCh] = out_1[i] + out_2[i];
    }
}

ctagSoundProcessorWTOscDuo::ctagSoundProcessorWTOscDuo() {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    lfo_1.SetSampleRate( 44100.f / bufSz);
    lfo_1.SetFrequency(1.f);

    lfo_2.SetSampleRate( 44100.f / bufSz);
    lfo_2.SetFrequency(1.f);
    // alloc mem for one wavetable
    
    buffer = (int16_t*)heap_caps_malloc(260 * 64 * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // 260 = wavetable size after prep, 64 wavetables, 2 bytes per sample (int16)
    assert(buffer != NULL);
    memset(buffer, 0, 260 * 64 * 2);
    fbuffer = (float*)heap_caps_malloc(512 * 4, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL); // buffer for wavetable prep computations
    assert(fbuffer != NULL);
    memset(fbuffer, 0, 512 * 4);
    
    oscillator_1.Init();
    oscillator_2.Init();
    svf_1.Init();
    svf_2.Init();
    
    adsr_1.SetModeExp();
    adsr_1.SetSampleRate(44100.f / bufSz);
    adsr_1.Reset();

    adsr_2.SetModeExp();
    adsr_2.SetSampleRate(44100.f / bufSz);
    adsr_2.Reset();
    pitchQuantizer.Init();
}

ctagSoundProcessorWTOscDuo::~ctagSoundProcessorWTOscDuo() 
{
    heap_caps_free(buffer);
    heap_caps_free(fbuffer);
}

void ctagSoundProcessorWTOscDuo::prepareWavetables()
{
    // precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
    // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
    // check if sample rom seems to have current bank
    if(!sample_rom_1.HasSliceGroup(currentBank * 64, currentBank * 64 + 63))
    {
        isWaveTableGood = false;
        return;
    }
    int size = sample_rom_1.GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
    if(size != 256*64)
    {
        isWaveTableGood = false;
        return;
    }
    int bankOffset = currentBank*64*256;
    int bufferOffset = 4*64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
    sample_rom_1.Read(&buffer[bufferOffset], bankOffset, 256 * 64);
    // start conversion of data
    // 64 wavetables per bank
    int c = 0;
    for(int i=0;i<64;i++){ // iterate all waves
        int startOffset = bufferOffset + i*256; // which wave
        // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
        float sum4 = buffer[startOffset] + buffer[startOffset + 1] + buffer[startOffset + 2] + buffer[startOffset + 3]; // add dc
        for(int j=0;j<512;j++){
            fbuffer[j] = buffer[startOffset + (j % 256)] + sum4;
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

void ctagSoundProcessorWTOscDuo::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("gain_1", [&](const int val){ gain_1 = val;});
	pMapCv.emplace("gain_1", [&](const int val){ cv_gain_1 = val;});
	pMapPar.emplace("gain_2", [&](const int val){ gain_2 = val;});
	pMapCv.emplace("gain_2", [&](const int val){ cv_gain_2 = val;});
	pMapPar.emplace("gate_1", [&](const int val){ gate_1 = val;});
	pMapTrig.emplace("gate_1", [&](const int val){ trig_gate_1 = val;});
	pMapPar.emplace("pitch_1", [&](const int val){ pitch_1 = val;});
	pMapCv.emplace("pitch_1", [&](const int val){ cv_pitch_1 = val;});
	pMapPar.emplace("gate_2", [&](const int val){ gate_2 = val;});
	pMapTrig.emplace("gate_2", [&](const int val){ trig_gate_2 = val;});
	pMapPar.emplace("pitch_2", [&](const int val){ pitch_2 = val;});
	pMapCv.emplace("pitch_2", [&](const int val){ cv_pitch_2 = val;});
	pMapPar.emplace("q_scale", [&](const int val){ q_scale = val;});
	pMapCv.emplace("q_scale", [&](const int val){ cv_q_scale = val;});
	pMapPar.emplace("tune_1", [&](const int val){ tune_1 = val;});
	pMapCv.emplace("tune_1", [&](const int val){ cv_tune_1 = val;});
	pMapPar.emplace("tune_2", [&](const int val){ tune_2 = val;});
	pMapCv.emplace("tune_2", [&](const int val){ cv_tune_2 = val;});
	pMapPar.emplace("wavebank", [&](const int val){ wavebank = val;});
	pMapCv.emplace("wavebank", [&](const int val){ cv_wavebank = val;});
	pMapPar.emplace("wave_1", [&](const int val){ wave_1 = val;});
	pMapCv.emplace("wave_1", [&](const int val){ cv_wave_1 = val;});
	pMapPar.emplace("wave_2", [&](const int val){ wave_2 = val;});
	pMapCv.emplace("wave_2", [&](const int val){ cv_wave_2 = val;});
	pMapPar.emplace("fmode", [&](const int val){ fmode = val;});
	pMapCv.emplace("fmode", [&](const int val){ cv_fmode = val;});
	pMapPar.emplace("fcut_1", [&](const int val){ fcut_1 = val;});
	pMapCv.emplace("fcut_1", [&](const int val){ cv_fcut_1 = val;});
	pMapPar.emplace("fcut_2", [&](const int val){ fcut_2 = val;});
	pMapCv.emplace("fcut_2", [&](const int val){ cv_fcut_2 = val;});
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
	pMapPar.emplace("vintage_vibe", [&](const int val){ vintage_vibe = val;});
	pMapCv.emplace("vintage_vibe", [&](const int val){ cv_vintage_vibe = val;});
	pMapPar.emplace("lfosync_1", [&](const int val){ lfosync_1 = val;});
	pMapTrig.emplace("lfosync_1", [&](const int val){ trig_lfosync_1 = val;});
	pMapPar.emplace("lfosync_2", [&](const int val){ lfosync_2 = val;});
	pMapTrig.emplace("lfosync_2", [&](const int val){ trig_lfosync_2 = val;});
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
	id = "WTOscDuo";
	// sectionCpp0
}