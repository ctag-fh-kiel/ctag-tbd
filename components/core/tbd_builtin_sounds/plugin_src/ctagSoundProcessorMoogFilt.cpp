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

#include <tbd/sounds/SoundProcessorMoogFilt.hpp>
#include <iostream>

using namespace tbd::sounds;

void SoundProcessorMoogFilt::Init(std::size_t blockSize, void *blockPtr) {
    ladderFilters[0] = &simplifiedMoog;
    ladderFilters[1] = &musicDSPMoog;
    ladderFilters[2] = &oberheimVariationMoog;
    ladderFilters[3] = &huovilainenMoog;
    ladderFilters[4] = &krajeskiMoog;
    ladderFilters[5] = &stilsonMoog;
/* these models are too much for the ESP without further optimizations
    ladderFilters[4] = std::make_unique<ImprovedMoog>(44100.f);
    ladderFilters[6] = std::make_unique<MicrotrackerMoog>(44100.f);
    ladderFilters[7] = std::make_unique<RKSimulationMoog>(44100.f);
*/

    adEnv.SetSampleRate(44100.f / bufSz);
    adEnv.SetModeExp();
}

void SoundProcessorMoogFilt::Process(const sound_processor::ProcessData&data) {
    float buf[bufSz];

    int mode = flt_model;
    if (mode > 5) mode = 5;
    if (mode < 0) mode = 0;

    // eg fm
    float fAttack, fDecay;
    float fAD = 0.f;
    if (enableEG == 1 && trig_enableEG != -1) {
        if (data.trig[trig_enableEG] != prevTrigState) {
            prevTrigState = data.trig[trig_enableEG];
            if (prevTrigState == 0) adEnv.Trigger();
        }
        adEnv.SetLoop(loopEG);
        fAttack = static_cast<float>(attack) / 4095.f;
        if (cv_attack != -1) {
            fAttack = fabsf(data.cv[cv_attack]);
        }
        fDecay = static_cast<float>(decay) / 4095.f;
        if (cv_decay != -1) {
            fDecay = fabsf(data.cv[cv_decay]);
        }
        adEnv.SetAttack(fAttack);
        adEnv.SetDecay(fDecay);
        fAD = adEnv.Process();
    }

    // modulation
    float fGain = static_cast<float>(gain) / 4095.f * fGainScales[mode];
    float fCutoff = static_cast<float>(cutoff) / 4095.f * fCutoffScales[mode];
    float fResonance = static_cast<float>(resonance) / 4095.f * fResoScales[mode];
    float fFM = static_cast<float>(fm_amt) / 4095.f;
    if (cv_cutoff != -1) {
        fCutoff = data.cv[cv_cutoff] * data.cv[cv_cutoff] * fCutoffScales[mode];
    }
    if (cv_resonance != -1) {
        fResonance = fabsf(data.cv[cv_resonance]) * fResoScales[mode];
    }
    if (cv_gain != -1) {
        fGain = fabsf(data.cv[cv_gain]) * fGainScales[mode];
    }
    if (cv_fm_amt != -1) {
        // external CV
        fCutoff += fFM * data.cv[cv_fm_amt] * fCutoffScales[mode];
    } else if (enableEG == 1) {
        // internal AD env
        fCutoff += fFM * fAD * fCutoffScales[mode];
    }

    // bounds checks
    if (fCutoff < 20.f) fCutoff = 20.f;
    if (fCutoff < 100.f && mode == 5) fCutoff = 100.f;
    if (fCutoff > fCutoffScales[mode]) fCutoff = fCutoffScales[mode];
    if (fResonance < 0.f) fResonance = 0.f;
    //if(fResonance > fResoScales[mode]) fResonance = fResoScales[mode];

    // create data structure
    for (int i = 0; i < bufSz; i++) {
        buf[i] = data.buf[i * 2 + this->processCh] * fGain;
    }

    // Process
    ladderFilters[mode]->SetCutoff(fCutoff);
    ladderFilters[mode]->SetResonance(fResonance);
    ladderFilters[mode]->Process(buf, bufSz);

    // write back
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2 + this->processCh] = buf[i];
    }
}
