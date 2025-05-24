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


#include <tbd/sound_processor.hpp>
#include <atomic>
#include <string>
#include <tbd/sound_utils/ctagWNoiseGen.hpp>
#include <tbd/sound_utils/ctagPNoiseGen.hpp>
#include <tbd/sound_utils/ctagADEnv.hpp>
#include <tbd/sound_utils/ctagBiQuad.hpp>

// based on
// https://www.musicdsp.org/en/latest/Synthesis/10-fast-sine-and-cosine-calculation.html

namespace tbd::sounds {

struct SoundProcessorHihat1 : sound_processor::SoundProcessor {

    void Process(const sound_processor::ProcessData&) override;
    virtual void Init(std::size_t blockSize, void *blockPtr) override;

protected:

    // process only variables
    float freq = 10.f;
    const float fs = 44100.f;
    float a = 0.f;
    float loud = 0.5f;
    float s[2];
    float preCVLoudness = 0.f;
    float preCVFrequency = 0.f;
    float attackVal = 0.f;
    float decayVal = 1.f;
    float attackVal_p = 0.f;
    float decayVal_p = 1.f;
    int prevTrigState = 1;
    int prevTrigState_p = 1;
    sound_utils::ctagADEnv adEnv, pitchEnv;
    sound_utils::ctagWNoiseGen wNoise;
    sound_utils::ctagPNoiseGen pNoise;
    sound_utils::ctagBiQuad filterBP;

    // sectionHpp
    std::atomic<int32_t> ntype, trig_ntype;
    std::atomic<int32_t> frequency, cv_frequency;
    std::atomic<int32_t> qfac, cv_qfac;
    std::atomic<int32_t> loudness, cv_loudness;
    std::atomic<int32_t> enableEG, trig_enableEG;
    std::atomic<int32_t> loopEG, trig_loopEG;
    std::atomic<int32_t> attack, cv_attack;
    std::atomic<int32_t> decay, cv_decay;
    std::atomic<int32_t> enableEG_p, trig_enableEG_p;
    std::atomic<int32_t> loopEG_p, trig_loopEG_p;
    std::atomic<int32_t> amount_p, cv_amount_p;
    std::atomic<int32_t> attack_p, cv_attack_p;
    std::atomic<int32_t> decay_p, cv_decay_p;
    // sectionHpp
};

}
