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
#include <cmath>
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagPNoiseGen.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "helpers/ctagADEnv.hpp"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorSubSynth : public ctagSoundProcessor {
        public:
            void Process(const ProcessData &) override;

           virtual void Init(std::size_t blockSize, void *blockPtr) override;

            const char *GetCStrID() const;

        private:
            virtual void knowYourself() override {};

            void setParamValueInternal(const string &id, const string &key, const int val) override;

            void loadPresetInternal() override;

// autogenerated code here
// sectionHpp
            const string id = "SubSynth";
            atomic<int32_t> srcsel, cv_srcsel;
            atomic<int32_t> cascade, cv_cascade;
            atomic<int32_t> partials, cv_partials;
            atomic<int32_t> gain, cv_gain;
            atomic<int32_t> enableEG, trig_enableEG;
            atomic<int32_t> attack, cv_attack;
            atomic<int32_t> decay, cv_decay;
            atomic<int32_t> sustain, cv_sustain;
            atomic<int32_t> release, cv_release;
            atomic<int32_t> root_frequency, cv_root_frequency;
            atomic<int32_t> root_bwidth, cv_root_bwidth;
            atomic<int32_t> root_level, cv_root_level;
            atomic<int32_t> p_harm[9];
            atomic<int32_t> cv_p_harm[9];
            atomic<int32_t> p_bwidth[9];
            atomic<int32_t> cv_p_bwidth[9];
            atomic<int32_t> p_gain[9];
            atomic<int32_t> cv_p_gain[9];
            atomic<int32_t> eg_enableEG[2];
            atomic<int32_t> trig_eg_enableEG[2];
            atomic<int32_t> eg_loopEG[2];
            atomic<int32_t> trig_eg_loopEG[2];
            atomic<int32_t> eg_mode[2];
            atomic<int32_t> trig_eg_mode[2];
            atomic<int32_t> eg_attack[2];
            atomic<int32_t> cv_eg_attack[2];
            atomic<int32_t> eg_decay[2];
            atomic<int32_t> cv_eg_decay[2];
            atomic<int32_t> p_modbwsrc[9];
            atomic<int32_t> p_modbw[9];
            atomic<int32_t> cv_p_modbw[9];
            atomic<int32_t> p_modgainsrc[9];
            atomic<int32_t> p_modgain[9];
            atomic<int32_t> cv_p_modgain[9];
            // sectionHpp
            float fBWidth[9] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fGain[9] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fHarm[9] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fModBW[9] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fModGain[9] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fRootBWidth {0.f};
            float fRootFrequency {0.f};
            float fRootLevel {0.f};
            const float fs = 44100.f;
            float filterZs[10 * 3][2]; // cascade 3 filters, base + 9 harmonics, biquad delay elements
            float fCoeffs[6] {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float fSumGain {1.f};
            HELPERS::ctagPNoiseGen pNoise;
            HELPERS::ctagWNoiseGen wNoise;
            HELPERS::ctagADSREnv adsrEnvSum;
            HELPERS::ctagADEnv eg[2];
            uint8_t prevTrigStateEG[2];
            uint8_t prevMode[2];

            void fetchControlData(const ProcessData &data);

            void updateEGs(const ProcessData &data);

            void computeFilterCoefs(float *coeffs, float freq, float bw, float gain);

            float computeRolloff(float freq);
        };
    }
}