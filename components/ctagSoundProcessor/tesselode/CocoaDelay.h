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

#pragma once

#include <cmath>
#include "Util.h"
#include "helpers/ctagSineSource.hpp"
#include "braids/svf.h"

namespace tesselode {

    const int numPrograms = 128;
    const int tapeLength = 10;

    enum class Parameters {
        delayTime,
        lfoAmount,
        lfoFrequency,
        driftAmount,
        driftSpeed,
        tempoSyncTime,
        feedback,
        stereoOffset,
        panMode,
        pan,
        duckAmount,
        duckAttackSpeed,
        duckReleaseSpeed,
        filterMode,
        lowCut,
        highCut,
        driveGain,
        driveMix,
        driveCutoff,
        driveIterations,
        dryVolume,
        wetVolume,
        numParameters
    };

    enum class TempoSyncTimes {
        tempoSyncOff,
        whole,
        dottedHalf,
        half,
        tripletHalf,
        dottedQuarter,
        quarter,
        tripletQuarter,
        dottedEighth,
        eighth,
        tripletEighth,
        dottedSixteenth,
        sixteenth,
        tripletSixteenth,
        dottedThirtysecond,
        thirtysecond,
        tripletThirtysecond,
        dottedSixtyforth,
        sixtyforth,
        tripletSixtyforth,
        numTempoSyncTimes
    };

    enum class PanModes {
        stationary,
        pingPong,
        circular,
        numPanModes
    };

    struct ParameterValues {
        bool isMono = false;
        float delayTime = 0.2f; //)->InitDouble("Delay time", .2, 0.001, 2.0, .01, "", "", 2.0);
        float lfoAmount = 0.f; // )->InitDouble("LFO amount", 0.0, 0.0, .5, .01, "", "", 2.0);
        float lfoFrequency = 2.f; //)->InitDouble("LFO frequency", 2.0, .1, 10.0, .01, "hz");
        float driftAmount = 0.001f;//)->InitDouble("Drift amount", .001, 0.0, .05, .01, "", "", 2.0);
        float driftSpeed = 1.f; //)->InitDouble("Drift speed", 1.0, .1, 10.0, .01, "", "", 2.0);
        TempoSyncTimes tempoSyncTime = TempoSyncTimes::tempoSyncOff; //)->InitEnum("Tempo sync delay time", (int)TempoSyncTimes::tempoSyncOff, (int)TempoSyncTimes::numTempoSyncTimes);
        float feedback = 0.5f; //)->InitDouble("Feedback amount", 0.5, -1.0, 1.0, .01);
        float stereoOffset = 0.0f; //)->InitDouble("Stereo offset", 0.0, -.5, .5, .01);
        PanModes panMode = PanModes::stationary; //)->InitEnum("Pan mode", (int)PanModes::stationary, (int)PanModes::numPanModes);
        float pan = 0.f; //)->InitDouble("Panning", 0.0, -pi * .5, pi * .5, .01);
        float duckAmount = 0.f; //)->InitDouble("Ducking amount", 0.0, 0.0, 10.0, .01, "", "", 1.0);
        float duckAttackSpeed = 10.f; //)->InitDouble("Ducking attack", 10.0, .1, 100.0, .01, "", "", 2.0);
        float duckReleaseSpeed = 10.f;//)->InitDouble("Ducking release", 10.0, .1, 100.0, .01, "", "", 2.0);
        braids::SvfMode filterMode = braids::SVF_MODE_LP;
        float svfCutoffFreq = 0.75;
        float svfResonance = 0.001f;
        float svfLfoFreq = 0.2f;
        float svfLfoAmt = 0.f;
        float svfMix = 0.f;
        float driveGain = 0.1f; //)->InitDouble("Drive amount", 0.1, 0.0, 10.0, .01, "", "", 2.0);
        float driveMix = 1.f; //)->InitDouble("Drive mix", 1.0, 0.0, 1.0, .01);
        float driveCutoff = 1.f; //)->InitDouble("Drive filter cutoff", 1.0, .01, 1.0, .01);
        int driveIterations = 1; //)->InitInt("Drive iterations", 1, 1, 16);
        float dryVolume = 1.f; //)->InitDouble("Dry volume", 1.0, 0.0, 2.0, .01);
        float wetVolume = 0.5f; //)->InitDouble("Wet volume", .5, 0.0, 2.0, .01);
        float bpm = 120.f;
    };

    class CocoaDelay {
    public:
        CocoaDelay();

        ~CocoaDelay();

        void Reset();

        void Process(float *data, int nFrames);

        float GetSampleRate();

        void SetSampleRate(const float &sampleRate);

        // effect parameters
        ParameterValues params;

    private:
        float GetDelayTime();

        void GetReadPositions(float &l, float &r);

        void InitBuffer();

        void UpdateReadPositions();

        void UpdateWritePosition();

        void UpdateParameters();

        void UpdateDucking(float input);

        void UpdateLfo();

        void UpdateDrift();

        float GetSample(float *buffer, float position);

        void WriteToBuffer(float *data, int s, float outL, float outR);

        float dt = 0.0;

        // parameters
        float fs = 44100.f;

        // delay
        /*
        std::vector<float> bufferL;
        std::vector<float> bufferR;
         */
        float *bufferL = NULL, *bufferR = NULL;
        int bufferSize = 0;
        int writePosition = 0;
        float readPositionL = 0.f;
        float readPositionR = 0.f;
        bool warmedUp = false;

        // fading parameters
        PanModes currentPanMode = PanModes::stationary;
        float parameterChangeVolume = 1.0;
        float stationaryPanAmount = 0.0;
        float circularPanAmount = 0.0;

        // filters
        braids::Svf svf_l, svf_r;

        // modulation
        float duckFollower = 0.0;
        float lfoPhase = 0.0;
        float driftVelocity = 0.0;
        float driftPhase = 0.0;
        ctagSineSource filtLfo;
    };
}

