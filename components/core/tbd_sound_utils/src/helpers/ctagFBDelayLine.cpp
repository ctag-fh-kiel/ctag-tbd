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


//
// Created by Robert Manzke on 10.02.20.
//

#include <cstring>
#include <tbd/sound_utils/ctagFBDelayLine.hpp>
#include <tbd/heaps.hpp>
#include <tbd/logging.hpp>

namespace heaps = tbd::heaps;

namespace tbd::sound_utils {

void ctagFBDelayLine::SetFeedback(float fb) {
    feedback = fb;
}

void ctagFBDelayLine::Process(float *samples, const uint32_t offset, const uint32_t inc,
                                                 const uint32_t size) {
    for (uint32_t i = 0; i < size; i += inc) {
        uint32_t wpos = (pos + len - 1) % len;
        uint32_t rpos = pos;
        buffer[wpos] = samples[i + offset] + feedback * buffer[wpos];
        samples[i + offset] = (1.f - drywet) * samples[i + offset] + drywet * buffer[rpos];
        pos++;
        pos %= len;
    }
}

ctagFBDelayLine::ctagFBDelayLine(uint32_t maxLength) {
    maxLen = maxLength;
    buffer = (float *) heaps::malloc(maxLength * sizeof(float), TBD_HEAPS_SPIRAM);
    if (buffer == nullptr) {
        TBD_LOGE("FBDLYLINE", "Out of memory!");
        return;
    }
    Clear();
}

ctagFBDelayLine::~ctagFBDelayLine() {
    heaps::free(buffer);
}

void ctagFBDelayLine::SetLength(const uint32_t length) {
    len = length > maxLen ? maxLen : length;
}

void ctagFBDelayLine::Clear() {
    memset((void *) buffer, 0, maxLen * sizeof(float));
}

void ctagFBDelayLine::SetDryWet(const float dw) {
    drywet = dw;
}

}
