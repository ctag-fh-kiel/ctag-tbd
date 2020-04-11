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
#include "ctagSoundProcessor.hpp"
extern "C" {
#include "gverb/gverb.h"
}

namespace CTAG::SP{
    class ctagSoundProcessorGVerb : public ctagSoundProcessor{
    public:
        void Process(const ProcessData &);
        ~ctagSoundProcessorGVerb();
        ctagSoundProcessorGVerb();
        const char * GetCStrID() const;
    private:
        void setParamValueInternal(const string &id, const string &key, const int val) override;
        void loadPresetInternal() override;
        const string id = "GVerb";
        float maxRoomSize = 500.f;
        // gverb objects
        ty_gverb *gverb;
        // inter thread variables are atomic
        atomic<int32_t> roomSz;
        atomic<int32_t> revTime;
        atomic<int32_t> damping;
        atomic<int32_t> inputBW;
        atomic<int32_t> earlyLvl;
        atomic<int32_t> tailLvl;
        atomic<int32_t> dryWet;
        atomic<int32_t> mono;
        atomic<int32_t> cvRoomSz;
        atomic<int32_t> cvRevTime;
        atomic<int32_t> cvDamping;
        atomic<int32_t> cvInputBW;
        atomic<int32_t> cvEarlyLvl;
        atomic<int32_t> cvTailLvl;
        atomic<int32_t> cvDryWet;
        // process only variables
        float fRoomSz, fRevTime, fDamping, fInputBW, fEarlyLvl, fTailLvl, fDryWet;
    };
}

