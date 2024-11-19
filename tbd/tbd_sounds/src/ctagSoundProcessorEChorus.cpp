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

#include <tbd/sounds/ctagSoundProcessorEChorus.hpp>
#include <cmath>

using namespace CTAG::SP;


void ctagSoundProcessorEChorus::Process(const ProcessData &data) {
    float fRange = pdepth / 4095.f;
    if (cv_pdepth != -1) {
        fRange = 0.05f * fabsf(data.cv[cv_pdepth]) + 0.95f * preRange;
        preRange = fRange;
    }
    echorus.SetRange(fRange);
    float fSpeed = prate / 4095.f;
    if (cv_prate != -1) {
        fSpeed = fabsf(data.cv[cv_prate]);
    }
    echorus.SetSpeed(fSpeed);
    float fWet = pwet / 4095.f;
    if (cv_pwet != -1) {
        fWet = fabs(data.cv[cv_pwet]);
    }
    echorus.SetWet(fWet);
    float fWidth = pwidth / 8192.f + 1.f;
    if (cv_pwidth != -1) {
        fWidth = fabsf(data.cv[cv_pwidth]) * 0.5f + 1.f;
    }
    echorus.SetWidth(fWidth);
    if (trig_mono != -1) {
        echorus.SetMono(data.trig[trig_mono] == 1 ? 0 : 1); // inverted logic
    } else {
        echorus.SetMono(mono);
    }
    if (cv_stages != -1) {
        echorus.SetStages(fabsf(data.cv[cv_stages] * 3.5f + 1.f));
    } else {
        echorus.SetStages(stages);
    }
    bool byp = bypass;
    if (trig_bypass != -1) {
        byp = data.trig[trig_bypass] == 1 ? 0 : 1;
    }
    echorus.SetBypass(byp);
    echorus.Process(data.buf, bufSz);
}

void ctagSoundProcessorEChorus::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);
}

ctagSoundProcessorEChorus::~ctagSoundProcessorEChorus() {
}

void ctagSoundProcessorEChorus::knowYourself() {
    // autogenerated code here
    // sectionCpp0
    pMapPar.emplace("bypass", [&](const int val) { bypass = val; });
    pMapTrig.emplace("bypass", [&](const int val) { trig_bypass = val; });
    pMapPar.emplace("pdepth", [&](const int val) { pdepth = val; });
    pMapCv.emplace("pdepth", [&](const int val) { cv_pdepth = val; });
    pMapPar.emplace("stages", [&](const int val) { stages = val; });
    pMapCv.emplace("stages", [&](const int val) { cv_stages = val; });
    pMapPar.emplace("prate", [&](const int val) { prate = val; });
    pMapCv.emplace("prate", [&](const int val) { cv_prate = val; });
    pMapPar.emplace("pwidth", [&](const int val) { pwidth = val; });
    pMapCv.emplace("pwidth", [&](const int val) { cv_pwidth = val; });
    pMapPar.emplace("pwet", [&](const int val) { pwet = val; });
    pMapCv.emplace("pwet", [&](const int val) { cv_pwet = val; });
    pMapPar.emplace("mono", [&](const int val) { mono = val; });
    pMapTrig.emplace("mono", [&](const int val) { trig_mono = val; });
    isStereo = true;
    id = "EChorus";
    // sectionCpp0
}