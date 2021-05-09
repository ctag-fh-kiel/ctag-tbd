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

#include <cstdint>

namespace CTAG::SP::HELPERS {
    class ctagFBDelayLine {
    public:
        ctagFBDelayLine(uint32_t maxLength);

        ~ctagFBDelayLine();

        void SetFeedback(const float fb);

        void SetLength(const uint32_t length);

        void SetDryWet(const float dw);

        void Clear();

        virtual void Process(float *samples, const uint32_t offset, const uint32_t inc, const uint32_t size);

    protected:
        float feedback = 0.f;
        float drywet = 1.f;
        float *buffer = nullptr;
        uint32_t pos = 0;
        uint32_t len = 0;
        uint32_t maxLen = 88200;
    };
}


