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
            void Process(const ProcessData &);

            ~ctagSoundProcessorDLoop();

            ctagSoundProcessorDLoop();

            const char *GetCStrID() const;

        private:
            void setParamValueInternal(const string &id, const string &key, const int val) override;
            void loadPresetInternal() override;
            const string id = "dloop"; // stands for dust looped

            // inter thread variables are atomic
            atomic<int32_t> reset;
            atomic<int32_t> loop;
            atomic<int32_t> seed;
            atomic<int32_t> level;
            atomic<int32_t> density;
            atomic<int32_t> slen;
            atomic<int32_t> sspread;
            atomic<int32_t> ofssspread;
            atomic<int32_t> vspread;
            atomic<int32_t> ofsvspread;
            atomic<int32_t> s_enable;
            atomic<int32_t> s_rlevel;
            atomic<int32_t> s_decay;
            atomic<int32_t> t_reset;
            atomic<int32_t> t_loop;
            atomic<int32_t> cv_seed;
            atomic<int32_t> cv_level;
            atomic<int32_t> cv_density;
            atomic<int32_t> cv_slen;
            atomic<int32_t> cv_sspread;
            atomic<int32_t> cv_vspread;
            atomic<int32_t> cv_ofssspread;
            atomic<int32_t> cv_ofsvspread;
            atomic<int32_t> t_s_enable;
            atomic<int32_t> cv_s_rlevel;
            atomic<int32_t> cv_s_decay;
            // process only objects
            ctagWNoiseGen d, d_pan, d_level, s_l, s_r;
            ctagDecay decL, decR;
            fv3::dccut_f dccutl, dccutr;
            uint32_t loopCntr;
            uint32_t lastReset;

        };
    }
}