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

#include <tbd/sounds/SoundProcessorCStripM.hpp>
#include <cmath>
#include <tbd/sound_utils/ctagFastMath.hpp>


using namespace tbd::sounds;

void SoundProcessorCStripM::Process(const sound_processor::ProcessData&data) {

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

void SoundProcessorCStripM::Init(std::size_t blockSize, void *blockPtr) {

}

SoundProcessorCStripM::~SoundProcessorCStripM() {

}
