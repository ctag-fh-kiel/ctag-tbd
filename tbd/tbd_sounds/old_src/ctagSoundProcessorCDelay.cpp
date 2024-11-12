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

#include "ctagSoundProcessorCDelay.hpp"
#include <iostream>
#include "braids/svf.h"

using namespace CTAG::SP;

void ctagSoundProcessorCDelay::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    delay.SetSampleRate(44100.f);
    delay.Reset();
}

void ctagSoundProcessorCDelay::Process(const ProcessData &data) {
    float dlyTime = dly_time / 1000.f;
    if (cv_dly_time != -1) {
        dlyTime = fabsf(data.cv[cv_dly_time]) * 1000.f;
    }
    delay.params.delayTime = dlyTime;

    float lfoAmount = lfo_amt / 4095.f * 0.5f;
    if (cv_lfo_amt != -1) {
        lfoAmount = fabsf(data.cv[cv_lfo_amt]) * 0.5f;
    }
    delay.params.lfoAmount = lfoAmount;

    float lfoFreq = lfo_frq / 4095.f * 10.f;
    if (cv_lfo_frq != -1) {
        lfoFreq = fabsf(data.cv[cv_lfo_frq]) * 10.f;
    }
    delay.params.lfoFrequency = lfoFreq;

    float lfoDriftAmt = lfo_drift_amt / 4095.f * 0.5f;
    if (cv_lfo_drift_amt != -1) {
        lfoDriftAmt = fabsf(data.cv[cv_lfo_drift_amt]) * 0.5f;
    }
    delay.params.driftAmount = lfoDriftAmt;

    float driftSpeed = lfo_drift_spd / 4095.f * 10.f;
    if (cv_lfo_drift_spd != -1) {
        driftSpeed = fabsf(data.cv[cv_lfo_drift_spd]) * 10.f;
    }
    delay.params.driftSpeed = driftSpeed;

    float feedback = dly_feedback / 4095.f * 1.2f;
    if (cv_dly_feedback != -1) {
        feedback = data.cv[cv_dly_feedback] * 1.2f;
    }
    if (freeze)feedback = 1.f;
    if (trig_freeze != -1) {
        if (data.trig[trig_freeze] == 0 ? 1 : 0) feedback = 1.f;
    }
    delay.params.feedback = feedback;

    float stereoOffset = dly_st_ofs / 4095.f * .5f;
    if (cv_dly_st_ofs != -1) {
        stereoOffset = data.cv[cv_dly_st_ofs] * .5f;
    }
    delay.params.stereoOffset = stereoOffset;

    float pan = dly_panning / 4095.f * M_PI * .5f;
    if (cv_dly_panning != -1) {
        pan = data.cv[cv_dly_panning] * M_PI * .5f;
    }
    delay.params.pan = pan;

    float duckAmount = duck_amt / 4095.f * 10.f;
    if (cv_duck_amt != -1) {
        duckAmount = fabsf(data.cv[cv_duck_amt]) * 10.f;
    }
    delay.params.duckAmount = duckAmount;

    float duckAttackSpeed = duck_atck / 4095.f * 100.f;
    if (cv_duck_atck != -1) {
        duckAttackSpeed = fabsf(data.cv[cv_duck_atck]) * 100.f;
    }
    delay.params.duckAttackSpeed = duckAttackSpeed;

    float duckReleaseSpeed = duck_rls / 4095.f * 100.f;
    if (cv_duck_rls != -1) {
        duckReleaseSpeed = fabsf(data.cv[cv_duck_rls]) * 100.f;
    }
    delay.params.duckReleaseSpeed = duckReleaseSpeed;

    float flt_cut = flt_co / 4095.f;
    if (cv_flt_co != -1) {
        flt_cut = fabsf(data.cv[cv_flt_co]);
    }
    delay.params.svfCutoffFreq = flt_cut;

    float resp = flt_reso / 4095.f * 10.f;
    if (cv_flt_reso != -1) {
        resp = fabsf(data.cv[cv_flt_reso] * 10.f);
    }
    delay.params.svfResonance = resp;

    float dryVolume = dly_dry / 4095.f * 2.f;
    if (cv_dly_dry != -1) {
        dryVolume = fabsf(data.cv[cv_dly_dry]) * 2.f;
    }
    delay.params.dryVolume = dryVolume;

    float wetVolume = dly_wet / 4095.f * 2.f;
    if (cv_dly_wet != -1) {
        wetVolume = fabsf(data.cv[cv_dly_wet]) * 2.f;
    }
    delay.params.wetVolume = wetVolume;

    int tempoSyncTime = dly_sync;
    if (cv_dly_sync != -1) {
        tempoSyncTime = static_cast<int>(fabsf(data.cv[cv_dly_sync]) * 19.f);
    }
    delay.params.tempoSyncTime = (TempoSyncTimes) tempoSyncTime;

    int panMode = dly_pan_mode;
    if (cv_dly_pan_mode != -1) {
        panMode = static_cast<int>(fabsf(data.cv[cv_dly_pan_mode]) * 3.f);
    }
    delay.params.panMode = (PanModes) panMode;

    int filterMode = flt_mode;
    if (cv_flt_mode != -1) {
        filterMode = static_cast<int>(fabsf(data.cv[cv_flt_mode]) * 4.f);
    }
    delay.params.filterMode = filterMode;


    float bpm = static_cast<float>(dly_time_bpm);
    if (cv_dly_time_bpm != -1) {
        bpm = fabsf(data.cv[cv_dly_time_bpm]) * 220.f + 30.f;
    }
    delay.params.bpm = bpm;

    delay.params.isMono = dly_mono;

    float filtLfoFreq = flt_lfo_freq / 4095.f * 20.f;
    if (cv_flt_lfo_freq != -1) {
        filtLfoFreq = fabsf(data.cv[cv_flt_lfo_freq]) * 20.f;
    }
    delay.params.svfLfoFreq = filtLfoFreq;

    float svfLfoAmt = flt_lfo_amt / 4095.f;
    if (cv_flt_lfo_amt != -1) {
        svfLfoAmt = data.cv[cv_flt_lfo_amt];
    }
    delay.params.svfLfoAmt = svfLfoAmt;

    float svfMix = flt_mix / 4095.f;
    if (cv_flt_mix != -1) {
        svfMix = data.cv[cv_flt_mix];
    }
    delay.params.svfMix = svfMix;

    delay.Process(data.buf, bufSz);
}

void ctagSoundProcessorCDelay::knowYourself() {
// autogenerated code here
// sectionCpp0
    pMapPar.emplace("dly_time", [&](const int val) { dly_time = val; });
    pMapCv.emplace("dly_time", [&](const int val) { cv_dly_time = val; });
    pMapPar.emplace("dly_time_bpm", [&](const int val) { dly_time_bpm = val; });
    pMapCv.emplace("dly_time_bpm", [&](const int val) { cv_dly_time_bpm = val; });
    pMapPar.emplace("dly_sync", [&](const int val) { dly_sync = val; });
    pMapCv.emplace("dly_sync", [&](const int val) { cv_dly_sync = val; });
    pMapPar.emplace("dly_feedback", [&](const int val) { dly_feedback = val; });
    pMapCv.emplace("dly_feedback", [&](const int val) { cv_dly_feedback = val; });
    pMapPar.emplace("freeze", [&](const int val) { freeze = val; });
    pMapTrig.emplace("freeze", [&](const int val) { trig_freeze = val; });
    pMapPar.emplace("dly_st_ofs", [&](const int val) { dly_st_ofs = val; });
    pMapCv.emplace("dly_st_ofs", [&](const int val) { cv_dly_st_ofs = val; });
    pMapPar.emplace("dly_pan_mode", [&](const int val) { dly_pan_mode = val; });
    pMapCv.emplace("dly_pan_mode", [&](const int val) { cv_dly_pan_mode = val; });
    pMapPar.emplace("dly_panning", [&](const int val) { dly_panning = val; });
    pMapCv.emplace("dly_panning", [&](const int val) { cv_dly_panning = val; });
    pMapPar.emplace("dly_mono", [&](const int val) { dly_mono = val; });
    pMapTrig.emplace("dly_mono", [&](const int val) { trig_dly_mono = val; });
    pMapPar.emplace("dly_wet", [&](const int val) { dly_wet = val; });
    pMapCv.emplace("dly_wet", [&](const int val) { cv_dly_wet = val; });
    pMapPar.emplace("dly_dry", [&](const int val) { dly_dry = val; });
    pMapCv.emplace("dly_dry", [&](const int val) { cv_dly_dry = val; });
    pMapPar.emplace("lfo_amt", [&](const int val) { lfo_amt = val; });
    pMapCv.emplace("lfo_amt", [&](const int val) { cv_lfo_amt = val; });
    pMapPar.emplace("lfo_frq", [&](const int val) { lfo_frq = val; });
    pMapCv.emplace("lfo_frq", [&](const int val) { cv_lfo_frq = val; });
    pMapPar.emplace("lfo_drift_amt", [&](const int val) { lfo_drift_amt = val; });
    pMapCv.emplace("lfo_drift_amt", [&](const int val) { cv_lfo_drift_amt = val; });
    pMapPar.emplace("lfo_drift_spd", [&](const int val) { lfo_drift_spd = val; });
    pMapCv.emplace("lfo_drift_spd", [&](const int val) { cv_lfo_drift_spd = val; });
    pMapPar.emplace("duck_amt", [&](const int val) { duck_amt = val; });
    pMapCv.emplace("duck_amt", [&](const int val) { cv_duck_amt = val; });
    pMapPar.emplace("duck_atck", [&](const int val) { duck_atck = val; });
    pMapCv.emplace("duck_atck", [&](const int val) { cv_duck_atck = val; });
    pMapPar.emplace("duck_rls", [&](const int val) { duck_rls = val; });
    pMapCv.emplace("duck_rls", [&](const int val) { cv_duck_rls = val; });
    pMapPar.emplace("flt_mode", [&](const int val) { flt_mode = val; });
    pMapCv.emplace("flt_mode", [&](const int val) { cv_flt_mode = val; });
    pMapPar.emplace("flt_co", [&](const int val) { flt_co = val; });
    pMapCv.emplace("flt_co", [&](const int val) { cv_flt_co = val; });
    pMapPar.emplace("flt_reso", [&](const int val) { flt_reso = val; });
    pMapCv.emplace("flt_reso", [&](const int val) { cv_flt_reso = val; });
    pMapPar.emplace("flt_lfo_freq", [&](const int val) { flt_lfo_freq = val; });
    pMapCv.emplace("flt_lfo_freq", [&](const int val) { cv_flt_lfo_freq = val; });
    pMapPar.emplace("flt_lfo_amt", [&](const int val) { flt_lfo_amt = val; });
    pMapCv.emplace("flt_lfo_amt", [&](const int val) { cv_flt_lfo_amt = val; });
    pMapPar.emplace("flt_mix", [&](const int val) { flt_mix = val; });
    pMapCv.emplace("flt_mix", [&](const int val) { cv_flt_mix = val; });
    isStereo = true;
    id = "CDelay";
    // sectionCpp0
}
