/*
MIT License

Copyright (c) 2018 Andrew Minnich https://github.com/tesselode

Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */

/* adapted to TBD and modified by RM */

#include <tbd/sound_utils/tesselode/CocoaDelay.h>
#include <tbd/sound_utils/ctagFastMath.hpp>
#include "stmlib/stmlib.h"
#include "stmlib/dsp/dsp.h"
#include <cstring>
#include <tbd/logging.hpp>
#include "stmlib/dsp/units.h"
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace tesselode;
using namespace tbd::sound_utils;

CocoaDelay::CocoaDelay() {
    svf_l.Init();
    svf_r.Init();
}

CocoaDelay::~CocoaDelay() {
    heaps::free(bufferL);
    heaps::free(bufferR);
}

float CocoaDelay::GetDelayTime() {
    float delayTime;
    float beatLength = 60.f / params.bpm;
    switch (params.tempoSyncTime) //(TempoSyncTimes)(int)params.tempoSyncTime)
    {
        case TempoSyncTimes::tempoSyncOff:
            delayTime = params.delayTime;//params.delayTime;
            break;
        case TempoSyncTimes::whole:
            delayTime = beatLength * 4.f;
            break;
        case TempoSyncTimes::dottedHalf:
            delayTime = beatLength * 3.f;
            break;
        case TempoSyncTimes::half:
            delayTime = beatLength * 2.f;
            break;
        case TempoSyncTimes::tripletHalf:
            delayTime = beatLength * 4.f / 3.f;
            break;
        case TempoSyncTimes::dottedQuarter:
            delayTime = beatLength * 3.f / 2.f;
            break;
        case TempoSyncTimes::quarter:
            delayTime = beatLength * 1.f;
            break;
        case TempoSyncTimes::tripletQuarter:
            delayTime = beatLength * 2.f / 3.f;
            break;
        case TempoSyncTimes::dottedEighth:
            delayTime = beatLength * 3.f / 4.f;
            break;
        case TempoSyncTimes::eighth:
            delayTime = beatLength * 1.f / 2.f;
            break;
        case TempoSyncTimes::tripletEighth:
            delayTime = beatLength * 1.f / 3.f;
            break;
        case TempoSyncTimes::dottedSixteenth:
            delayTime = beatLength * 3.f / 8.f;
            break;
        case TempoSyncTimes::sixteenth:
            delayTime = beatLength * 1.f / 4.f;
            break;
        case TempoSyncTimes::tripletSixteenth:
            delayTime = beatLength * 1.f / 6.f;
            break;
        case TempoSyncTimes::dottedThirtysecond:
            delayTime = beatLength * 3.f / 16.f;
            break;
        case TempoSyncTimes::thirtysecond:
            delayTime = beatLength * 1.f / 8.f;
            break;
        case TempoSyncTimes::tripletThirtysecond:
            delayTime = beatLength * 1.f / 12.f;
            break;
        case TempoSyncTimes::dottedSixtyforth:
            delayTime = beatLength * 3.f / 32.f;
            break;
        case TempoSyncTimes::sixtyforth:
            delayTime = beatLength * 1.f / 16.f;
            break;
        case TempoSyncTimes::tripletSixtyforth:
            delayTime = beatLength * 1.f / 24.f;
            break;
        default:
            delayTime = params.delayTime;
            break;
    }

    // modulation
    float lfoAmount = params.lfoAmount; //params.lfoAmount;
    if (lfoAmount != 0.0f) delayTime = tbd::sound_utils::powf_fast(delayTime, 1.0f + lfoAmount * fastsin(lfoPhase * 2.f * pi));
    float driftAmount = params.driftAmount; //params.driftAmount;
    if (driftAmount != 0.0f) delayTime = tbd::sound_utils::powf_fast(delayTime, 1.0f + driftAmount * fastsin(driftPhase));

    return delayTime;
}

void CocoaDelay::GetReadPositions(float &l, float &r) {
    float offset = params.stereoOffset * 0.5f;
    float baseTime = GetDelayTime();
    float timeL = tbd::sound_utils::powf_fast(baseTime, 1.0f + offset);
    float timeR = tbd::sound_utils::powf_fast(baseTime, 1.0f - offset);
    l = timeL * GetSampleRate();
    r = timeR * GetSampleRate();
}

void CocoaDelay::InitBuffer() {
    /*
    bufferL.resize(GetSampleRate() * tapeLength);
    bufferR.resize(GetSampleRate() * tapeLength);
    std::fill(bufferL.begin(), bufferL.end(), 0.f);
    std::fill(bufferR.begin(), bufferR.end(), 0.f);
     */
    bufferL = (float *) heaps::malloc(sizeof(float) * GetSampleRate() * tapeLength, TBD_HEAPS_SPIRAM);
    if(bufferL == NULL){
        TBD_LOGE("CDelay", "Could not allocate buffer L in SPIRAM!");
        return;
    }
    memset(bufferL, 0, sizeof(float) * GetSampleRate() * tapeLength);
    bufferR = (float *) heaps::malloc(sizeof(float) * GetSampleRate() * tapeLength, TBD_HEAPS_SPIRAM);
    if(bufferL == NULL){
        TBD_LOGE("CDelay", "Could not allocate buffer R in SPIRAM!");
        return;
    }
    memset(bufferR, 0, sizeof(float) * GetSampleRate() * tapeLength);
    bufferSize = GetSampleRate() * tapeLength;
    writePosition = 0.0f;
    GetReadPositions(readPositionL, readPositionR);
}

void CocoaDelay::UpdateReadPositions() {
    float targetReadPositionL, targetReadPositionR;
    GetReadPositions(targetReadPositionL, targetReadPositionR);
    readPositionL += (targetReadPositionL - readPositionL) * 10.0f * dt;
    readPositionR += (targetReadPositionR - readPositionR) * 10.0f * dt;
}

void CocoaDelay::UpdateWritePosition() {
    writePosition += 1;
    writePosition %= bufferSize;
}

void CocoaDelay::UpdateParameters() {
    // pan mode fadeout
    if (currentPanMode != (PanModes) (int) params.panMode) {
        parameterChangeVolume -= 100.0f * dt;
        if (parameterChangeVolume <= 0.0f) {
            parameterChangeVolume = 0.0f;
            currentPanMode = (PanModes) (int) params.panMode;
        }
    } else if (parameterChangeVolume < 1.0f) {
        parameterChangeVolume += 100.0f * dt;
        if (parameterChangeVolume > 1.0f) parameterChangeVolume = 1.0f;
    }

    // pan amount smoothing
    float panAmount = params.pan;
    float stationaryPanAmountTarget = (currentPanMode == PanModes::stationary || currentPanMode == PanModes::pingPong)
                                      ? panAmount : 0.0f;
    stationaryPanAmount += (stationaryPanAmountTarget - stationaryPanAmount) * 100.0f * dt;
    float circularPanAmountTarget = (currentPanMode == PanModes::circular ? panAmount : 0.0f);
    circularPanAmount += (circularPanAmountTarget - circularPanAmount) * 100.0f * dt;
}

void CocoaDelay::UpdateDucking(float input) {
    float attackSpeed = params.duckAttackSpeed;
    float releaseSpeed = params.duckReleaseSpeed;
    float speed = duckFollower < fabsf(input) ? attackSpeed : releaseSpeed;
    duckFollower += (fabsf(input) - duckFollower) * speed * dt;
}

void CocoaDelay::UpdateLfo() {
    lfoPhase += params.lfoFrequency * dt;
    while (lfoPhase > 1.0f) lfoPhase -= 1.0f;
}

void CocoaDelay::UpdateDrift() {
    float driftSpeed = params.driftSpeed;
    driftVelocity += random() * 10000.0f * driftSpeed * dt;
    driftVelocity -= driftVelocity * 2.0f * fastsqrt(driftSpeed) * dt;
    driftPhase += driftVelocity * dt;
}

float CocoaDelay::GetSample(float *buffer, float position) {
    int p0 = wrap(floorf(position) - 1, 0, bufferSize - 1);
    int p1 = wrap(floorf(position), 0, bufferSize - 1);
    int p2 = wrap(ceilf(position), 0, bufferSize - 1);
    int p3 = wrap(ceilf(position) + 1, 0, bufferSize - 1);

    float x = position - floorf(position);
    float y0 = buffer[p0];
    float y1 = buffer[p1];
    float y2 = buffer[p2];
    float y3 = buffer[p3];

    return interpolate(x, y0, y1, y2, y3);
}

void CocoaDelay::WriteToBuffer(float *data, int s, float outL, float outR) {
    float writeL = 0.0f, writeR = 0.0f;
    if (params.isMono) {
        writeL += data[s * 2];
        writeR += data[s * 2];
    } else {
        writeL += data[s * 2];
        writeR += data[s * 2 + 1];
    }

    adjustPanning(writeL, writeR, stationaryPanAmount * .5, writeL, writeR);

    writeL += outL * params.feedback;
    writeL = stmlib::SoftLimit(writeL);
    writeR += outR * params.feedback;
    writeR = stmlib::SoftLimit(writeR);
    switch (currentPanMode) {
        case PanModes::pingPong:
            bufferL[writePosition] = writeR * parameterChangeVolume;
            bufferR[writePosition] = writeL * parameterChangeVolume;
            break;
        default:
            bufferL[writePosition] = writeL * parameterChangeVolume;
            bufferR[writePosition] = writeR * parameterChangeVolume;
            break;
    }
}

void CocoaDelay::Process(float *data, int nFrames) {
    filtLfo.SetFrequency(params.svfLfoFreq);
    for (int s = 0; s < nFrames; s++) {
        // workaround for daws like renoise that don't start processing until the effect receives an input.
        // if it's the first sample to be processed, the read positions will be immediately set to their targets.
        // this way, when you first start a project, each delay plugin doesn't have to "slide up" to the correct delay time.
        if (!warmedUp) {
            GetReadPositions(readPositionL, readPositionR);
            warmedUp = true;
        }

        UpdateParameters();
        UpdateReadPositions();
        UpdateDucking(data[s * 2] + data[s * 2 + 1]);
        UpdateLfo();
        UpdateDrift();

        // read from buffer
        float outL = GetSample(bufferL, writePosition - readPositionL);
        float outR = GetSample(bufferR, writePosition - readPositionR);

        // circular panning
        adjustPanning(outL, outR, circularPanAmount, outL, outR);

        // filters
        float fLfo = filtLfo.Process();
        if ((int) params.filterMode != 0) {
            float fCut = params.svfCutoffFreq + params.svfLfoAmt * fLfo; // TODO: Pitch tracking
            CONSTRAIN(fCut, 0.f, 1.f)
            fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
            float fReso = params.svfResonance;
            CONSTRAIN(fReso, 1.f, 20.f)
            svf_l.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
            svf_r.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
            float tl = 0.f;
            float tr = 0.f;
            switch (params.filterMode) {
                case 1:
                    tl = svf_l.Process<stmlib::FILTER_MODE_LOW_PASS>(outL);
                    tr = svf_r.Process<stmlib::FILTER_MODE_LOW_PASS>(outR);
                    break;
                case 2:
                    tl = svf_l.Process<stmlib::FILTER_MODE_BAND_PASS>(outL);
                    tr = svf_r.Process<stmlib::FILTER_MODE_BAND_PASS>(outR);
                    break;
                case 3:
                    tl = svf_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(outL);
                    break;
                default:
                    break;
            }
            outL = (1.f - params.svfMix) * outL + params.svfMix * tl;
            outR = (1.f - params.svfMix) * outR + params.svfMix * tr;
        }

        // write to buffer
        WriteToBuffer(data, s, outL, outR);
        UpdateWritePosition();

        // output
        float dry = params.dryVolume;
        float wet = params.wetVolume;
        float duckValue = params.duckAmount * duckFollower;
        duckValue = duckValue > 1.0 ? 1.0 : duckValue;
        wet *= 1.0 - duckValue;
        data[s * 2] = data[s * 2] * dry + outL * wet;
        data[s * 2 + 1] = data[s * 2 + 1] * dry + outR * wet;
    }
}

void CocoaDelay::Reset() {
    dt = 1.0f / GetSampleRate();
    InitBuffer();
}

void CocoaDelay::SetSampleRate(const float &sampleRate) {
    fs = sampleRate;
    filtLfo.SetSampleRate(fs);
    filtLfo.SetFrequency(1.f);
}

float CocoaDelay::GetSampleRate() {
    return fs;
}


