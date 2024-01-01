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


#include "ctagSoundProcessorFBDlyLine.hpp"
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP;

void ctagSoundProcessorFBDlyLine::Init(std::size_t const &blockSize, void *const blockPtr){
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
}

void ctagSoundProcessorFBDlyLine::Process(const ProcessData &data) {
    if (trig_enable != -1) {
        if (data.trig[trig_enable] == 1 && enable == 1) return;
        else if (data.trig[trig_enable] == 0 && enable == 0) return;
    } else {
        if (enable == 0) return;
    }
    float fb = feedback / 4095.f;
    if (cv_feedback != -1) {
        fb += data.cv[cv_feedback] - 0.5f;
        if (fb > 1.f) fb = 1.f;
        if (fb < 0.f) fb = 0.f;
    }
    cvLength = (float) length;
    if (cv_length != -1) {
        cvLength += (data.cv[cv_length] - 0.5) * (float) (maxLength / 2);
    }
    fLength = 0.5f * fLength + 0.5f * cvLength;
    MK_FLT_PAR(fDryWet, drywet, 4095.f, 1.f)
    if (fLength > maxLength) fLength = maxLength;
    if (fLength < 1) fLength = 1.f;
    dlyLine.SetDryWet(fDryWet);
    dlyLine.SetLength((uint32_t) fLength);
    dlyLine.SetFeedback(fb);
    dlyLine.Process(data.buf, this->processCh, 2, 32 * 2);

    fLevel = (float) level / 4095.f;
    if (cv_level != -1) {
        fLevel += data.cv[cv_level] - 0.5f;
        if (fLevel < 0.f) fLevel = 0.f;
        if (fLevel > 1.f) fLevel = 1.f;
    }

    for (uint32_t i = 0; i < bufSz; i++) {
        data.buf[i * 2 + processCh] *= fLevel;
    }
}

void ctagSoundProcessorFBDlyLine::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("length", [&](const int val) { length = val; });
    pMapCv.emplace("length", [&](const int val) { cv_length = val; });
    pMapPar.emplace("feedback", [&](const int val) { feedback = val; });
    pMapCv.emplace("feedback", [&](const int val) { cv_feedback = val; });
    pMapPar.emplace("drywet", [&](const int val) { drywet = val; });
    pMapCv.emplace("drywet", [&](const int val) { cv_drywet = val; });
    pMapPar.emplace("level", [&](const int val) { level = val; });
    pMapCv.emplace("level", [&](const int val) { cv_level = val; });
    pMapPar.emplace("enable", [&](const int val) { enable = val; });
    pMapTrig.emplace("enable", [&](const int val) { trig_enable = val; });
    isStereo = false;
    id = "FBDlyLine";
    // sectionCpp0
}

