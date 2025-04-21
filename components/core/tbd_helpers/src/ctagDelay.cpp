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
#include "tbd/helpers/ctagDelay.hpp"

void CTAG::SP::HELPERS::ctagDelay::Mute() {
    memset(buffer, 0, bufsize * sizeof(float));
    bufidx = 0;
}

void CTAG::SP::HELPERS::ctagDelay::SetFeedback(float fb) {
    feedback = fb;
}

float CTAG::SP::HELPERS::ctagDelay::GetFeedback() {
    return feedback;
}

uint32_t CTAG::SP::HELPERS::ctagDelay::GetSize() {
    return bufsize;
}

float CTAG::SP::HELPERS::ctagDelay::GetLast() {
    return buffer[bufidx];
}

float CTAG::SP::HELPERS::ctagDelay::GetZ(uint32_t index) {
    return 0;
}

float CTAG::SP::HELPERS::ctagDelay::Process(float input) {
    float bufout = buffer[bufidx];
    buffer[bufidx] = input;
    bufidx++;
    bufidx %= bufsize;
    return bufout;
}

float CTAG::SP::HELPERS::ctagDelay::ProcessFeedback(float input) {
    float bufout = buffer[bufidx];
    buffer[bufidx] = feedback * input;
    bufidx++;
    bufidx %= bufsize;
    return bufout;
}

void CTAG::SP::HELPERS::ctagDelay::SetBuffer(float *buf, const uint32_t bufsz) {
    buffer = buf;
    bufsize = bufsz;
    bufidx = 0;
    Mute();
}

CTAG::SP::HELPERS::ctagDelay::ctagDelay() {
    feedback = 0.f;
    bufidx = 0;
}
