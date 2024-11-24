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
#include <cmath>

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorStrampDly : public ctagSoundProcessor {
        public:
            void Process(const ProcessData &) override;

            ~ctagSoundProcessorStrampDly();

           virtual void Init(std::size_t blockSize, void *blockPtr) override;

        private:
            virtual void knowYourself() override;

            void mute();

// autogenerated code here
// sectionHpp
           std::atomic<int32_t> mode, trig_mode;
           std::atomic<int32_t> freeze, trig_freeze;
           std::atomic<int32_t> bypass, trig_bypass;
           std::atomic<int32_t> length, cv_length;
           std::atomic<int32_t> feedback, cv_feedback;
           std::atomic<int32_t> pan, cv_pan;
           std::atomic<int32_t> wvol, cv_wvol;
           std::atomic<int32_t> dvol, cv_dvol;
           std::atomic<int32_t> gain, cv_gain;
            // sectionHpp




            float *bufL, *bufR;
            uint32_t bufLen;
            uint32_t pos;
            float sampleRate, msMaxLength, msLength;
            float fFeedback, fPan, fWetVolume, fDryVolume;
            uint32_t tapOffset;
            float fTapOffset;
            const float mFac = 4.f / M_PI;
            float envFollowBuffer = 0.f;
            float envFollowInput = 0.f;

        };
    }
}