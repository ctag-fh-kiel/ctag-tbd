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
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagEnvFollow.hpp"
#include "helpers/ctagDecay.hpp"
#include "freeverb3/efilter.hpp"
#include <atomic>
#include <string>

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDLoop : public ctagSoundProcessor {
        public:
            void Process(const ProcessData &) override;

           virtual void Init(std::size_t const &blockSize, void *const blockPtr) override;


        private:

            virtual void knowYourself() override;

            // process only objects
            ctagWNoiseGen d, d_pan, d_level, s_l, s_r;
            ctagDecay decL, decR;
            fv3::dccut_f dccutl, dccutr;
            uint32_t loopCntr;
            uint32_t lastReset;

            // sectionHpp
            atomic<int32_t> reset, trig_reset;
            atomic<int32_t> loop, trig_loop;
            atomic<int32_t> seed, cv_seed;
            atomic<int32_t> level, cv_level;
            atomic<int32_t> density, cv_density;
            atomic<int32_t> slen, cv_slen;
            atomic<int32_t> sspread, cv_sspread;
            atomic<int32_t> ofssspread, cv_ofssspread;
            atomic<int32_t> vspread, cv_vspread;
            atomic<int32_t> ofsvspread, cv_ofsvspread;
            atomic<int32_t> s_enable, trig_s_enable;
            atomic<int32_t> s_rlevel, cv_s_rlevel;
            atomic<int32_t> s_decay, cv_s_decay;
            // sectionHpp
        };
    }
}