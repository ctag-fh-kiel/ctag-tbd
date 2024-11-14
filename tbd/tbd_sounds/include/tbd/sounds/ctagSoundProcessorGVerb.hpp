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


#pragma once

#include <atomic>
#include <tbd/sound_processor.hpp>

extern "C" {
#include "gverb/gverb.h"
}

namespace CTAG::SP {
    class ctagSoundProcessorGVerb : public ctagSoundProcessor {
    public:
        void Process(const ProcessData &) override;

       virtual void Init(std::size_t blockSize, void *blockPtr) override;

        ~ctagSoundProcessorGVerb() override;

    private:
        virtual void knowYourself() override;

        float maxRoomSize = 500.f;
        // gverb objects
        ty_gverb *gverb;
        // process only variables
        float fRoomSz, fRevTime, fDamping, fInputBW, fEarlyLvl, fTailLvl, fDryWet;
        // sectionHpp
       std::atomic<int32_t> roomsize, cv_roomsize;
       std::atomic<int32_t> revtime, cv_revtime;
       std::atomic<int32_t> damping, cv_damping;
       std::atomic<int32_t> inputbw, cv_inputbw;
       std::atomic<int32_t> earlylvl, cv_earlylvl;
       std::atomic<int32_t> taillvl, cv_taillvl;
       std::atomic<int32_t> drywet, cv_drywet;
       std::atomic<int32_t> mono, trig_mono;
        // sectionHpp
    };
}

