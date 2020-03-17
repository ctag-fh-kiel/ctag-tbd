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
#include "helpers/ctagFBDelayLine.hpp"


namespace CTAG{
    namespace SP{
        class ctagSoundProcessorFBDlyLine: public ctagSoundProcessor{
            public: 
                void Process(const ProcessData &);
                ~ctagSoundProcessorFBDlyLine();
                ctagSoundProcessorFBDlyLine();
                const char * GetCStrID() const;
            private:
                void setParamValueInternal(const string &id, const string &key, const int val) override;
                void loadPresetInternal() override;
                const string id = "fbdlyline";
                const uint32_t maxLength;
                HELPERS::ctagFBDelayLine dlyLine;
                // inter thread variables are atomic
                atomic<int32_t> feedback;
                atomic<int32_t> length;
                atomic<int32_t> enable;
                atomic<int32_t> drywet;
                atomic<int32_t> level;
                atomic<int32_t> cvControlFeedback;
                atomic<int32_t> cvControlLength;
                atomic<int32_t> cvControlDryWet;
                atomic<int32_t> cvControlLevel;
                atomic<int32_t> trigControlEnable;
                // process only variables
                float fLength = 0.f, cvLength = 0.f;
                float fLevel = 1.f;
                float fDryWet = 0.5f;
        };
    }
}