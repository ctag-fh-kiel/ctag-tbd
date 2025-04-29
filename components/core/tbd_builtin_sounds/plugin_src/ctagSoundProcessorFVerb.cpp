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

#include <tbd/sounds/SoundProcessorFVerb.hpp>
#include <iostream>
#include <cmath>

using namespace tbd::sounds;

void SoundProcessorFVerb::Init(std::size_t blockSize, void *blockPtr) {
    // sets the block memory and initializes it, size check is in init
    freeverb.init(blockSize, blockPtr);
}

void SoundProcessorFVerb::Process(const audio::ProcessData&data) {
    float val = (float) roomsize / 4095.f;
    if (cv_roomsize != -1) val = data.cv[cv_roomsize] * data.cv[cv_roomsize];
    freeverb.setroomsize(val);
    val = (float) damp / 4095.f;
    if (cv_damp != -1) val = data.cv[cv_damp] * data.cv[cv_damp];
    freeverb.setdamp(val);
    val = (float) dry / 4095.f;
    if (cv_dry != -1) val = data.cv[cv_dry] * data.cv[cv_dry];
    freeverb.setdry(val);
    val = (float) width / 4095.f;
    if (cv_width != -1) val = data.cv[cv_width] * data.cv[cv_width];
    freeverb.setwidth(val);
    val = (float) mode;
    if (trig_mode != -1) {
        val = 1.f - data.trig[trig_mode];
    }
    freeverb.setmode(val);
    freeverb.setmono(mono);
    val = (float) wet / 4095.f;
    if (cv_wet != -1) val = data.cv[cv_wet] * data.cv[cv_wet];
    freeverb.setwet(val);
    freeverb.process(data.buf, bufSz);
}
