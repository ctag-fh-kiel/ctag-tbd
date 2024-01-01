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

#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "plaits/dsp/voice.h"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorTBDaits : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;

            virtual ~ctagSoundProcessorTBDaits();

           virtual void Init(std::size_t const &blockSize, void *const blockPtr) override;


        private:
            virtual void knowYourself() override;

            // private attributes could go here
            plaits::Voice voice;
            plaits::Patch patch = {};
            char *shared_buffer;
            // sectionHpp
            atomic<int32_t> smodel, cv_smodel;
            atomic<int32_t> trigger, trig_trigger;
            atomic<int32_t> frequency, cv_frequency;
            atomic<int32_t> level, cv_level;
            atomic<int32_t> harmonics, cv_harmonics;
            atomic<int32_t> timbre, cv_timbre;
            atomic<int32_t> morph, cv_morph;
            atomic<int32_t> lpg_color, cv_lpg_color;
            atomic<int32_t> lpg_decay, cv_lpg_decay;
            atomic<int32_t> mod_freq, cv_mod_freq;
            atomic<int32_t> mod_timbre, cv_mod_timbre;
            atomic<int32_t> mod_morph, cv_mod_morph;
            // sectionHpp
        };
    }
}