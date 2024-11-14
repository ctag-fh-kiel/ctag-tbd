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
#include "helpers/ctagFBDelayLine.hpp"


namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFBDlyLine : public ctagSoundProcessor {
        public:
            void Process(const ProcessData &) override;

           virtual void Init(std::size_t blockSize, void *blockPtr) override;

        private:

            virtual void knowYourself() override;

            const uint32_t maxLength {88200};
            HELPERS::ctagFBDelayLine dlyLine {maxLength};
            // process only variables
            float fLength = 0.f, cvLength = 0.f;
            float fLevel = 1.f;
            // sectionHpp
           std::atomic<int32_t> length, cv_length;
           std::atomic<int32_t> feedback, cv_feedback;
           std::atomic<int32_t> drywet, cv_drywet;
           std::atomic<int32_t> level, cv_level;
           std::atomic<int32_t> enable, trig_enable;
            // sectionHpp
        };
    }
}