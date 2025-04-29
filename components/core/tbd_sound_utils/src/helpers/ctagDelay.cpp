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
// Created by Robert Manzke on 13.02.20.
//

#include <cstring>
#include <tbd/sound_utils/ctagDelay.hpp>

namespace tbd::sound_utils {

void ctagDelay::Mute() {
    memset(buffer, 0, bufsize * sizeof(float));
    bufidx = 0;
}

void ctagDelay::SetFeedback(float fb) {
    feedback = fb;
}

float ctagDelay::GetFeedback() {
    return feedback;
}

uint32_t ctagDelay::GetSize() {
    return bufsize;
}

float ctagDelay::GetLast() {
    return buffer[bufidx];
}

float ctagDelay::GetZ(uint32_t index) {
    return 0;
}

float ctagDelay::Process(float input) {
    float bufout = buffer[bufidx];
    buffer[bufidx] = input;
    bufidx++;
    bufidx %= bufsize;
    return bufout;
}

float ctagDelay::ProcessFeedback(float input) {
    float bufout = buffer[bufidx];
    buffer[bufidx] = feedback * input;
    bufidx++;
    bufidx %= bufsize;
    return bufout;
}

void ctagDelay::SetBuffer(float *buf, const uint32_t bufsz) {
    buffer = buf;
    bufsize = bufsz;
    bufidx = 0;
    Mute();
}

ctagDelay::ctagDelay() {
    feedback = 0.f;
    bufidx = 0;
}

}
