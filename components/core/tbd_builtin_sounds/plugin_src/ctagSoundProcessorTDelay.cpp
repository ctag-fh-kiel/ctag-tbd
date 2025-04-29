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

#include <tbd/sounds/SoundProcessorTDelay.hpp>
#include <cmath>

using namespace tbd::sounds;

void SoundProcessorTDelay::Process(const audio::ProcessData&data) {
    float fDelay = delay / 4095.f;
    if(cv_delay != -1){
        fDelay = 0.05f * fabsf(data.cv[cv_delay]) + 0.95f * preDelay;
        preDelay = fDelay;
    }
    tdelay.SetDelay(fDelay);
    float fWet = wet / 4095.f;
    if(cv_wet != -1){
        fWet = fabsf(data.cv[cv_wet]);
    }
    tdelay.SetWet(fWet);
    float fDry = dry / 4095.f;
    if(cv_dry != -1){
        fDry = fabsf(data.cv[cv_dry]);
    }
    tdelay.SetDry(fDry);
    float fFeedback = feedback / 4095.f;
    if(cv_feedback != -1){
        fFeedback = fabsf(data.cv[cv_feedback]);
    }
    tdelay.SetFeedback(fFeedback);
    float fLNFT = lnft / 4095.f;
    if(cv_lnft != -1){
        fLNFT = 0.05f * fabsf(data.cv[cv_lnft]) + 0.95f * preLNFT;
        preLNFT = fLNFT;
    }
    tdelay.SetLNFT(fLNFT);
    float fDepth = depth / 4095.f;
    if(cv_depth != -1){
        fDepth = 0.05f * fabsf(data.cv[cv_depth]) + 0.95f * preDepth;
        preDepth = fDepth;
    }
    tdelay.SetDepth(fDepth);
    bool bByp = bypass;
    if(trig_bypass != -1){
        bByp = data.trig[trig_bypass] == 1 ? 0 : 1;
    }
    tdelay.SetBypass(bByp);
    tdelay.Process(data.buf, bufSz, processCh);
}

void SoundProcessorTDelay::Init(std::size_t blockSize, void *blockPtr) {
    assert(blockSize > 258*4); // tdelay memory requirements
    tdelay.SetBlockMem(blockPtr);
}

SoundProcessorTDelay::~SoundProcessorTDelay() {
}
