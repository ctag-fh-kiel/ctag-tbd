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

#include <tbd/sounds/SoundProcessorEveryTrim.hpp>
#include <cmath>
#include <tbd/sound_utils/ctagFastMath.hpp>


using namespace tbd::sounds;

void SoundProcessorEveryTrim::Process(const sound_processor::ProcessData&data) {

    float fLeft = left / 4095.f;
    if (cv_left != -1) {
        fLeft = data.cv[cv_left]; // range 0 ..1 or -1 .. 1
    }
    everytrim.SetLeft(fLeft);

    float fRight = right / 4095.f;
    if (cv_right != -1) {
        fRight = data.cv[cv_right]; // range 0 ..1 or -1 .. 1
    }
    everytrim.SetRight(fRight);

    float fMid = mid / 4095.f;
    if (cv_mid != -1) {
        fMid = data.cv[cv_mid]; // range 0 ..1 or -1 .. 1
    }
    everytrim.SetMid(fMid);

    float fSide = side / 4095.f;
    if (cv_side != -1) {
        fSide = data.cv[cv_side]; // range 0 ..1 or -1 .. 1
    }
    everytrim.SetSide(fSide);

    float fMaster = master / 4095.f;
    if (cv_master != -1) {
        fMaster = data.cv[cv_master]; // range 0 ..1 or -1 .. 1
    }
    everytrim.SetMaster(fMaster);

    everytrim.Process(data.buf, bufSz);

}

void SoundProcessorEveryTrim::Init(std::size_t blockSize, void *blockPtr) {

}

SoundProcessorEveryTrim::~SoundProcessorEveryTrim() {

}
