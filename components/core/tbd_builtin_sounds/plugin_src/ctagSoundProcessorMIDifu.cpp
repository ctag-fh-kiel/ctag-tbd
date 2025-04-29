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

#include <tbd/sounds/SoundProcessorMIDifu.hpp>
#include <iostream>
#include <tbd/sound_utils/ctagFastMath.hpp>

using namespace tbd::sounds;

void SoundProcessorMIDifu::Init(std::size_t blockSize, void *blockPtr) {
    assert(blockSize >= 8192 * sizeof(float));
    fx_buffer = (float *) blockPtr;
    fx.Init(fx_buffer);
    fx.Clear();
}

void SoundProcessorMIDifu::Process(const audio::ProcessData&data) {
    float fTime = time / 4095.f;
    float fAmount = amount / 4095.f;

    if (cv_time != -1) {
        fTime = sound_utils::fastfabs(data.cv[cv_time]);
    }
    if (cv_amount != -1) {
        fAmount = sound_utils::fastfabs(data.cv[cv_amount]);
    }

    float io[bufSz];
    for (int i = 0; i < bufSz; i++) {
        io[i] = data.buf[i * 2 + processCh];
    }
    fx.Process(fAmount, fTime, io, bufSz);
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] = io[i];
    }
}

SoundProcessorMIDifu::~SoundProcessorMIDifu() {
}
