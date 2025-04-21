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


#include <tbd/sounds/ctagSoundProcessorStrampDly.hpp>
#include <iostream>
#include <cmath>
#include "tbd/helpers/ctagFastMath.hpp"
#include <tbd/logging.hpp>
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace CTAG::SP;

void ctagSoundProcessorStrampDly::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);

    // config defs
    msMaxLength = 4095.f;
    fFeedback = 0.f;
    msLength = length;
    sampleRate = 44100.f;
    bufLen = ceilf(sampleRate * msMaxLength / 1000.0);
    bufL = (float *) heaps::calloc(bufLen, sizeof(float), TBD_HEAPS_SPIRAM); // mono
    if (bufL == NULL) {
        TBD_LOGE("DELAY", "Could not allocate memory --> delay buffer!");
    }
    bufR = (float *) heaps::calloc(bufLen, sizeof(float), TBD_HEAPS_SPIRAM); // mono
    if (bufR == NULL) {
        TBD_LOGE("DELAY", "Could not allocate memory --> delay buffer!");
    }
    mute();

}

void ctagSoundProcessorStrampDly::Process(const ProcessData &data) {
    float tempL, tempR;
    if (cv_length != -1) {
        msLength = data.cv[cv_length] * data.cv[cv_length] * msMaxLength;
    } else {
        msLength = (float) length / 4095.f * msMaxLength;
    }

    fTapOffset = 0.995 * fTapOffset + 0.005 * (msMaxLength - msLength) * sampleRate / 1000.0;
    tapOffset = (uint32_t) fTapOffset;
    if (cv_feedback != -1) {
        fFeedback = 0.8 * fFeedback + 0.2 * data.cv[cv_feedback] * data.cv[cv_feedback] * 2.f;
    } else {
        fFeedback = 0.8 * fFeedback + 0.2 * (float) feedback / 4095.f * 1.5f;
    }
    if (trig_freeze != -1) {
        if (data.trig[trig_freeze] == 0) fFeedback = 1.f; // inverted ins
    } else {
        if (freeze == 1)
            fFeedback = 1.f;
    }

    if (cv_pan != -1)
        fPan = data.cv[cv_pan] * data.cv[cv_pan];
    else
        fPan = (float) pan / 4095.f;
    if (cv_wvol != -1)
        fWetVolume = data.cv[cv_wvol] * data.cv[cv_wvol];
    else
        fWetVolume = (float) wvol / 4095.f;
    if (cv_dvol != -1)
        fDryVolume = data.cv[cv_dvol] * data.cv[cv_dvol];
    else
        fDryVolume = (float) dvol / 4095.f;
    float fGain;
    if (cv_gain != -1)
        fGain = data.cv[cv_gain] * data.cv[cv_gain];
    else
        fGain = (float) gain / 4095.f * 2.f;

    if (trig_bypass != -1) {
        if (data.trig[trig_bypass] == 0) return;
    } else {
        if (bypass == 1) return;
    }

    uint32_t dlyMode = mode;
    if (trig_mode != -1) dlyMode = data.trig[trig_mode] == 1 ? 0 : 1; // inverted trig signals

    for (uint32_t i = 0; i < bufSz; i++) {
        uint32_t cPos;
        // read
        cPos = (pos + tapOffset) % bufLen;
        tempL = bufL[cPos];
        tempR = bufR[cPos];

        // write
        cPos = pos;
        if (dlyMode == 1) {
            bufL[cPos] = tempR * fFeedback;
            bufR[cPos] = tempL * fFeedback;
        } else {
            bufL[cPos] = tempL * fFeedback;
            bufR[cPos] = tempR * fFeedback;
        }

        // noise reduction with envelope follower input
        if (fabs(data.buf[i * 2]) > envFollowInput)
            envFollowInput = fabs(data.buf[i * 2]);
        else if (fabs(data.buf[i * 2 + 1]) > envFollowInput)
            envFollowInput = fabs(data.buf[i * 2] + 1);
        else
            envFollowInput *= 0.9f; // decay

        if (envFollowInput > 0.0001) {
            bufL[cPos] += (data.buf[i * 2] + data.buf[i * 2 + 1]) * (1.f - fPan) * 2.f;
            bufR[cPos] += (data.buf[i * 2] + data.buf[i * 2 + 1]) * fPan * 2.f;
        }


        // noise reduction with envelope follower buffer
        if (fabs(bufL[cPos]) > envFollowBuffer)
            envFollowBuffer = fabs(bufL[cPos]);
        else if (fabs(bufR[cPos]) > envFollowBuffer)
            envFollowBuffer = fabs(bufR[cPos]);
        else
            envFollowBuffer *= 0.9f; // decay

        if (envFollowBuffer < 0.0001) {
            bufL[cPos] = 0.f;
            bufR[cPos] = 0.f;
        }

        // update position
        pos++;
        pos %= bufLen;

        // set volume
        data.buf[i * 2] *= fDryVolume;
        data.buf[i * 2] += tempL * fWetVolume;
        data.buf[i * 2 + 1] *= fDryVolume;
        data.buf[i * 2 + 1] += tempR * fWetVolume;

        data.buf[i * 2] = mFac * HELPERS::fasttanh(data.buf[i * 2] * fGain);
        data.buf[i * 2 + 1] = mFac * HELPERS::fasttanh(data.buf[i * 2 + 1] * fGain);
    }
}

ctagSoundProcessorStrampDly::~ctagSoundProcessorStrampDly() {
    heaps::free(bufL);
    heaps::free(bufR);
}

void ctagSoundProcessorStrampDly::mute() {
    memset(bufL, 0, bufLen * sizeof(float));
    memset(bufR, 0, bufLen * sizeof(float));
    tapOffset = (uint32_t) (sampleRate * (msMaxLength - msLength) / 1000.0);
    fTapOffset = tapOffset;
    pos = 0;
}

void ctagSoundProcessorStrampDly::knowYourself() {
// sectionCpp0
    pMapPar.emplace("mode", [&](const int val) { mode = val; });
    pMapTrig.emplace("mode", [&](const int val) { trig_mode = val; });
    pMapPar.emplace("freeze", [&](const int val) { freeze = val; });
    pMapTrig.emplace("freeze", [&](const int val) { trig_freeze = val; });
    pMapPar.emplace("bypass", [&](const int val) { bypass = val; });
    pMapTrig.emplace("bypass", [&](const int val) { trig_bypass = val; });
    pMapPar.emplace("length", [&](const int val) { length = val; });
    pMapCv.emplace("length", [&](const int val) { cv_length = val; });
    pMapPar.emplace("feedback", [&](const int val) { feedback = val; });
    pMapCv.emplace("feedback", [&](const int val) { cv_feedback = val; });
    pMapPar.emplace("pan", [&](const int val) { pan = val; });
    pMapCv.emplace("pan", [&](const int val) { cv_pan = val; });
    pMapPar.emplace("wvol", [&](const int val) { wvol = val; });
    pMapCv.emplace("wvol", [&](const int val) { cv_wvol = val; });
    pMapPar.emplace("dvol", [&](const int val) { dvol = val; });
    pMapCv.emplace("dvol", [&](const int val) { cv_dvol = val; });
    pMapPar.emplace("gain", [&](const int val) { gain = val; });
    pMapCv.emplace("gain", [&](const int val) { cv_gain = val; });
    isStereo = true;
    id = "StrampDly";
    // sectionCpp0
}