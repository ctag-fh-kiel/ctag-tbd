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


#include <tbd/sounds/SoundProcessorHihat1.hpp>
#include <iostream>
#include <cmath>
#include <tbd/sound_utils/ctagFastMath.hpp>


using namespace tbd::sounds;

void SoundProcessorHihat1::Init(std::size_t blockSize, void *blockPtr) {
    // init params
    wNoise.SetBipolar(true);
    filterBP.SetType(sound_utils::BIQUAD_TYPE::BP);
    filterBP.SetQ(0.75f);
    // ad envs
    adEnv.SetSampleRate(44100.f);
    adEnv.SetModeExp();
    pitchEnv.SetSampleRate(44100.f / (float) bufSz);
    pitchEnv.SetModeLin();
}

void SoundProcessorHihat1::Process(const sound_processor::ProcessData&data) {
    float deltaCVLoud = (data.cv[cv_loudness] - preCVLoudness) / (float) bufSz; // for linear CV interpolation
    freq = (float) frequency; // frequency
    // cv frequency
    if (cv_frequency != -1) {
        preCVFrequency = 0.3f * data.cv[cv_frequency] + 0.7f * preCVFrequency; // smooth CV
        float fMod = preCVFrequency * 5.f;
        fMod = tbd::sound_utils::fastpow2(fMod);
        freq *= fMod;
    }
    // eg pitch
    if (enableEG_p == 1 && trig_enableEG_p != -1) {
        if (data.trig[trig_enableEG_p] != prevTrigState_p) {
            prevTrigState_p = data.trig[trig_enableEG_p];
            if (prevTrigState_p == 0) pitchEnv.Trigger();
        }
        pitchEnv.SetLoop(loopEG_p);
        attackVal_p = (float) attack_p / 4095.f;
        if (cv_attack_p != -1) {
            attackVal_p = data.cv[cv_attack_p] * data.cv[cv_attack_p];
        }
        decayVal_p = (float) decay_p / 4095.f;
        if (cv_decay_p != -1) {
            decayVal_p = data.cv[cv_decay_p] * data.cv[cv_decay_p];
        }
        pitchEnv.SetAttack(attackVal_p);
        pitchEnv.SetDecay(decayVal_p);
    }
    // parameter control
    loud = (float) loudness / 4095.f;
    //  eg loud
    if (enableEG == 1 && trig_enableEG != -1) {
        if (data.trig[trig_enableEG] != prevTrigState) {
            prevTrigState = data.trig[trig_enableEG];
            if (prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        attackVal = (float) attack / 4095.f;
        if (cv_attack != -1) {
            attackVal = data.cv[cv_attack] * data.cv[cv_attack];
        }
        decayVal = (float) decay / 4095.f;
        if (cv_decay != -1) {
            decayVal = data.cv[cv_decay] * data.cv[cv_decay];
        }
        adEnv.SetAttack(attackVal);
        adEnv.SetDecay(decayVal);
    }
    // here is the oscillator
    float freqb = freq;
    float tmp[bufSz];
    int nt = ntype;
    if (trig_ntype != -1) nt = data.trig[trig_ntype];
    for (int i = 0; i < bufSz; i++) {
        if (nt == 0)
            tmp[i] = wNoise.Process();
        else if (nt == 1)
            tmp[i] = pNoise.Process();
    }
    if (enableEG_p == 1 && trig_enableEG_p != -1) {
        float val;
        if (cv_amount_p == -1)
            val = tbd::sound_utils::fasterpow2(pitchEnv.Process() * (float) amount_p / 4095.f * 8.f);
        else
            val = tbd::sound_utils::fasterpow2(pitchEnv.Process() * data.cv[cv_amount_p] * 8.f);
        freqb *= val;
        if (freqb > 20000.f) freqb = 20000.f;
        if (freqb < 15.f) freqb = 15.f;
    }
    float q = (float) qfac / 4095.f * 100.f;
    if (cv_qfac != -1) {
        q = data.cv[cv_qfac];
        q *= q;
        q *= 100.f;
    }
    filterBP.SetQ(q);
    filterBP.SetCutoffHz(freqb);
    filterBP.Process(tmp, bufSz);
    // apply loudness
    for (int i = 0; i < this->bufSz; i++) { // iterate all channel samples
        data.buf[i * 2 + this->processCh] = tmp[i] * loud;
        // apply loud EG
        if (enableEG == 1) {
            data.buf[i * 2 + this->processCh] *= adEnv.Process();
        }
        // apply CV loud control
        if (cv_loudness != -1) {
            data.buf[i * 2 + this->processCh] *=
                    preCVLoudness * preCVLoudness; // linearly interpolate CV data, square for loudness perception
            preCVLoudness += deltaCVLoud;
        }
    }
}
