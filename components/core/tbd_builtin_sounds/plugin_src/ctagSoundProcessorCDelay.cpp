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

#include <tbd/sounds/SoundProcessorCDelay.hpp>
#include <iostream>
#include "braids/svf.h"

using namespace tbd::sounds;

void SoundProcessorCDelay::Init(std::size_t blockSize, void *blockPtr) {
    delay.SetSampleRate(44100.f);
    delay.Reset();
}

void SoundProcessorCDelay::Process(const sound_processor::ProcessData&data) {
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
