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
#include "helpers/ctagGNoiseGen.hpp"
#include "helpers/ctagDustGen.hpp"

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDust : public ctagSoundProcessor {
        public:
            void Process(const ProcessData &) override;

           virtual void Init() override;

        private:

            virtual void knowYourself() override;

            // process only variables

            // random sources
            ctagDustGen dust;

            // filter coefs
            float coeffs[6];
            float w1[2];
            float w2[2];

            // sectionHpp
            atomic<int32_t> bipolar, trig_bipolar;
            atomic<int32_t> rate, cv_rate;
            atomic<int32_t> level, cv_level;
            atomic<int32_t> width, cv_width;
            atomic<int32_t> smooth, cv_smooth;
            atomic<int32_t> bp_enable, trig_bp_enable;
            atomic<int32_t> bp_fcut, cv_bp_fcut;
            atomic<int32_t> bp_q, cv_bp_q;
            // sectionHpp
        };
    }
}