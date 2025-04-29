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
#include <tbd/sound_utils/ctagGNoiseGen.hpp>
#include <tbd/sound_utils/ctagDustGen.hpp>

using namespace tbd::sound_utils;

namespace tbd::sounds {
    
struct SoundProcessorDust : audio::SoundProcessor {

    void Process(const audio::ProcessData&) override;

    virtual void Init(std::size_t blockSize, void *blockPtr) override;

protected:

    // process only variables

    // random sources
    ctagDustGen dust;

    // filter coefs
    float coeffs[6];
    float w1[2];
    float w2[2];

    // sectionHpp
    std::atomic<int32_t> bipolar, trig_bipolar;
    std::atomic<int32_t> rate, cv_rate;
    std::atomic<int32_t> level, cv_level;
    std::atomic<int32_t> width, cv_width;
    std::atomic<int32_t> smooth, cv_smooth;
    std::atomic<int32_t> bp_enable, trig_bp_enable;
    std::atomic<int32_t> bp_fcut, cv_bp_fcut;
    std::atomic<int32_t> bp_q, cv_bp_q;
    // sectionHpp
};

}
