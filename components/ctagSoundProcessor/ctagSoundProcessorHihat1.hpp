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


#include "ctagSoundProcessor.hpp"
#include <atomic>
#include <string>
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagPNoiseGen.hpp"
#include "helpers/ctagADEnv.hpp"
#include "helpers/ctagBiQuad.hpp"

// based on
// https://www.musicdsp.org/en/latest/Synthesis/10-fast-sine-and-cosine-calculation.html

namespace CTAG{
    namespace SP{
        class ctagSoundProcessorHihat1 : public ctagSoundProcessor{
            public: 
                void Process(const ProcessData &);
                ~ctagSoundProcessorHihat1();
                ctagSoundProcessorHihat1();
                const char * GetCStrID() const;
            private:
                void setParamValueInternal(const string &id, const string &key, const int val) override;
                void loadPresetInternal() override;
                const string id = "Hihat1";
                // inter thread variables are atomic
                atomic<int32_t> ntype;
                atomic<int32_t> loudness;
                atomic<int32_t> qfac;
                atomic<int32_t> frequency;
                atomic<int32_t> trigNType;
                atomic<int32_t> cvControlLoudness;
                atomic<int32_t> cvControlQFac;
                atomic<int32_t> cvControlFrequency;
                atomic<int32_t> attack;
                atomic<int32_t> decay;
                atomic<int32_t> enableEG;
                atomic<int32_t> cvControlAttack;
                atomic<int32_t> cvControlDecay;
                atomic<int32_t> trigEG;
                atomic<int32_t> loopEG;
                atomic<int32_t> attack_p;
                atomic<int32_t> decay_p;
                atomic<int32_t> enableEG_p;
                atomic<int32_t> amount_p;
                atomic<int32_t> cvControlAttack_p;
                atomic<int32_t> cvControlDecay_p;
                atomic<int32_t> cvAmount_p;
                atomic<int32_t> trigEG_p;
                atomic<int32_t> loopEG_p;
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
                HELPERS::ctagADEnv adEnv, pitchEnv;
                HELPERS::ctagWNoiseGen wNoise;
                HELPERS::ctagPNoiseGen pNoise;
                HELPERS::ctagBiQuad filterBP;
        };
    }
}