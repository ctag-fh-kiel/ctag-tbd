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

#include <tbd/sounds/ctagSoundProcessorCStripM.hpp>
#include <cmath>
#include "helpers/ctagFastMath.hpp"


using namespace CTAG::SP;

void ctagSoundProcessorCStripM::Process(const ProcessData &data) {

    float fTreble = treble / 4095.f;
    if (cv_treble != -1) {
        fTreble = data.cv[cv_treble]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetTreble(fTreble);

    float fMid = mid / 4095.f;
    if (cv_mid != -1) {
        fMid = data.cv[cv_mid]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetMid(fMid);

    float fBass = bass / 4095.f;
    if (cv_bass != -1) {
        fBass = data.cv[cv_bass]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetBass(fBass);

    float fLowpass = lowpass / 4095.f;
    if (cv_lowpass != -1) {
        fLowpass = data.cv[cv_lowpass]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetLowpass(fLowpass);

    float fTrebfreq = trebfreq / 4095.f;
    if (cv_trebfreq != -1) {
        fTrebfreq = data.cv[cv_trebfreq]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetTrebfreq(fTrebfreq);

    float fBassfreq = bassfreq / 4095.f;
    if (cv_bassfreq != -1) {
        fBassfreq = data.cv[cv_bassfreq]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetBassfreq(fBassfreq);

    float fHipass = hipass / 4095.f;
    if (cv_hipass != -1) {
        fHipass = data.cv[cv_hipass]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetHipass(fHipass);

    float fGate = gate / 4095.f;
    if (cv_gate != -1) {
        fGate = data.cv[cv_gate]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetGate(fGate);

    float fComp = comp / 4095.f;
    if (cv_comp != -1) {
        fComp = data.cv[cv_comp]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetComp(fComp);

    float fCompspd = compspd / 4095.f;
    if (cv_compspd != -1) {
        fCompspd = data.cv[cv_compspd]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetCompspd(fCompspd);

    float fOutgain = outgain / 4095.f;
    if (cv_outgain != -1) {
        fOutgain = data.cv[cv_outgain]; // range 0 ..1 or -1 .. 1
    }
    CStripM.SetOutgain(fOutgain);

    CStripM.Process(data.buf, bufSz, processCh);

}

void ctagSoundProcessorCStripM::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);
}

ctagSoundProcessorCStripM::~ctagSoundProcessorCStripM() {
}

void ctagSoundProcessorCStripM::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("treble", [&](const int val){ treble = val;});
	pMapCv.emplace("treble", [&](const int val){ cv_treble = val;});
	pMapPar.emplace("mid", [&](const int val){ mid = val;});
	pMapCv.emplace("mid", [&](const int val){ cv_mid = val;});
	pMapPar.emplace("bass", [&](const int val){ bass = val;});
	pMapCv.emplace("bass", [&](const int val){ cv_bass = val;});
	pMapPar.emplace("lowpass", [&](const int val){ lowpass = val;});
	pMapCv.emplace("lowpass", [&](const int val){ cv_lowpass = val;});
	pMapPar.emplace("trebfreq", [&](const int val){ trebfreq = val;});
	pMapCv.emplace("trebfreq", [&](const int val){ cv_trebfreq = val;});
	pMapPar.emplace("bassfreq", [&](const int val){ bassfreq = val;});
	pMapCv.emplace("bassfreq", [&](const int val){ cv_bassfreq = val;});
	pMapPar.emplace("hipass", [&](const int val){ hipass = val;});
	pMapCv.emplace("hipass", [&](const int val){ cv_hipass = val;});
	pMapPar.emplace("gate", [&](const int val){ gate = val;});
	pMapCv.emplace("gate", [&](const int val){ cv_gate = val;});
	pMapPar.emplace("comp", [&](const int val){ comp = val;});
	pMapCv.emplace("comp", [&](const int val){ cv_comp = val;});
	pMapPar.emplace("compspd", [&](const int val){ compspd = val;});
	pMapCv.emplace("compspd", [&](const int val){ cv_compspd = val;});
	pMapPar.emplace("timelag", [&](const int val){ timelag = val;});
	pMapCv.emplace("timelag", [&](const int val){ cv_timelag = val;});
	pMapPar.emplace("outgain", [&](const int val){ outgain = val;});
	pMapCv.emplace("outgain", [&](const int val){ cv_outgain = val;});
	isStereo = false;
	id = "CStripM";
	// sectionCpp0
}