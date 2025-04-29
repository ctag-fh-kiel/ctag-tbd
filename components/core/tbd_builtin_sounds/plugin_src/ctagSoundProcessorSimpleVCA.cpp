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

#include <tbd/sounds/SoundProcessorSimpleVCA.hpp>
#include <iostream>


using namespace tbd::sounds;

void SoundProcessorSimpleVCA::Init(std::size_t blockSize, void *blockPtr) {

}

void SoundProcessorSimpleVCA::Process(const audio::ProcessData&data) {
    float fLoudness = loudness / 4095.f;
    if (cv_loudness != -1) {
        fLoudness = data.cv[cv_loudness]; // range 0 ..1 or -1 .. 1
    }

    for (int i = 0; i < this->bufSz; i++) {
        data.buf[i * 2 + this->processCh] = fLoudness * data.buf[i * 2 + this->processCh];
    }
}

SoundProcessorSimpleVCA::~SoundProcessorSimpleVCA() {
}
