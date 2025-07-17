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

#include <tbd/sounds/SoundProcessorMIEnsemble.hpp>
#include <iostream>
#include <tbd/sound_utils/ctagFastMath.hpp>

using namespace tbd::sounds;

void SoundProcessorMIEnsemble::Init(std::size_t blockSize, void *blockPtr) {
    assert(blockSize >= 4096 * sizeof(float));
    fx_buffer = (float *) blockPtr;

    fx.Init(fx_buffer);
}

void SoundProcessorMIEnsemble::Process(const sound_processor::ProcessData&data) {
    float fAmount = amount / 4095.f;
    float fDepth = depth / 4095.f;
    if (cv_amount != -1) {
        fAmount = sound_utils::fastfabs(data.cv[cv_amount]);
    }
    if (cv_depth != -1) {
        fDepth = sound_utils::fastfabs(data.cv[cv_depth]);
    }
    fx.set_depth(fDepth);
    fx.set_amount(fAmount);

    float left[bufSz], right[bufSz];
    for (int i = 0; i < bufSz; i++) {
        left[i] = data.buf[i * 2];
        right[i] = data.buf[i * 2 + 1];
    }

    fx.Process(left, right, bufSz);

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = left[i];
        data.buf[i * 2 + 1] = right[i];
    }
}

SoundProcessorMIEnsemble::~SoundProcessorMIEnsemble() {
}
