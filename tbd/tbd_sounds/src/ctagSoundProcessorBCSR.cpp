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

#include <tbd/sounds/ctagSoundProcessorBCSR.hpp>
#include <iostream>
#include <cmath>
#include "helpers/ctagFastMath.hpp"

using namespace CTAG::SP;

#define MAX_GAIN 20.f

void ctagSoundProcessorBCSR::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);

    // inits
    prevSample = 0.f;
    phase = 0.f;
    egSR.SetSampleRate(44100.f);
    egSR.SetModeLin();
    egBC.SetSampleRate(44100.f);
    egBC.SetModeLin();
    bcEGTrig = true;
    srEGTrig = true;
}

// adapted from https://www.musicdsp.org/en/latest/Effects/139-lo-fi-crusher.html

void ctagSoundProcessorBCSR::Process(const ProcessData &data) {
    updateParams(data);
    // process values
    for (int i = 0; i < bufSz; ++i) {
        float out;

        if (isSRReduce) {
            float dPhase = fNormFreq + egSR.Process() * fEGSRAmount;
            if (dPhase < 1.f / 44100.f) dPhase = 1.f / 44100.f;
            if (dPhase > 0.5f) fNormFreq = 1.f;
            phase += dPhase;
            if (phase >= 1.f) {
                phase -= 1.f;
                prevSample = data.buf[i * 2 + processCh];
            }
            out = prevSample;
        } else {
            out = data.buf[i * 2 + processCh];
        }

        out *= fGain;

        if (isBitCrush) {
            float crush = egBC.Process() * fEGBCAmount + fScale;
            if (crush < 0.f) crush = 0.f;
            if (crush > 16.f) crush = 16.f;
            crush = HELPERS::fastpow2(crush);
            if (HELPERS::fastfabs(out) < 1.f / crush) out = 0.f;
            out = floorf(out * crush) / crush;
        }

        out = HELPERS::fasttanh(out); // saturate

        if (isInvertWet) out *= -1.f;

        data.buf[i * 2 + processCh] = out + data.buf[i * 2 + processCh] * fDry;
    }
}


void ctagSoundProcessorBCSR::updateParams(const ProcessData &data) {
    // Sample rate reduce params
    if (cv_sr_amount != -1) {
        fNormFreq = 0.51f * (1.f - HELPERS::fastfabs(data.cv[cv_sr_amount]));
    } else {
        fNormFreq = 0.51f * (4095 - sr_amount) / 4095.f;
    }
    if (trig_sr_ena != -1) {
        isSRReduce = data.trig[trig_sr_ena] != 1;
    } else {
        isSRReduce = sr_ena;
    }

    // Bit crush params
    if (cv_bc_amount != -1) {
        fScale = 16.f - HELPERS::fastfabs(data.cv[cv_bc_amount]) * 15.f;
    } else {
        fScale = 16.f - bc_amount / 4095.f * 15.f;
    }
    if (trig_bc_ena != -1) {
        isBitCrush = data.trig[trig_bc_ena] != 1;
    } else {
        isBitCrush = bc_ena;
    }

    // gain params
    if (cv_level != -1) {
        fGain = data.cv[cv_level] * data.cv[cv_level] * MAX_GAIN;
    } else {
        fGain = level / 4095.f * MAX_GAIN;
    }
    if (cv_dry != -1) {
        fDry = data.cv[cv_dry] * data.cv[cv_dry];
    } else {
        fDry = dry / 4095.f;
    }
    if (trig_invert != -1) {
        isInvertWet = data.trig[trig_invert] != 1;
    } else {
        isInvertWet = invert;
    }

    // BC envelope
    if (trig_eg_bc_ena != -1) {
        if (data.trig[trig_eg_bc_ena] != bcEGTrig) {
            bcEGTrig = data.trig[trig_eg_bc_ena];
            if (!bcEGTrig) egBC.Trigger();
        }
    } else {
        if (eg_bc_ena != bcEGTrig) {
            bcEGTrig = eg_bc_ena;
            if (bcEGTrig) egBC.Trigger();
        }
    }
    if (trig_eg_bc_loop != -1) {
        egBC.SetLoop(data.trig[trig_eg_bc_loop] != 1);
    } else {
        egBC.SetLoop(eg_bc_loop);
    }

    if (cv_eg_bc_amount != -1) {
        fEGBCAmount = -8.f * data.cv[cv_eg_bc_amount];
    } else {
        fEGBCAmount = eg_bc_amount / 4095.f * -8.f;
    }
    if (cv_eg_bc_att != -1) {
        egBC.SetAttack(data.cv[cv_eg_bc_att] * data.cv[cv_eg_bc_att]);
    } else {
        egBC.SetAttack(eg_bc_att / 4095.f);
    }
    if (cv_eg_bc_dec != -1) {
        egBC.SetDecay(data.cv[cv_eg_bc_dec] * data.cv[cv_eg_bc_dec]);
    } else {
        egBC.SetDecay(eg_bc_dec / 4095.f);
    }
    if (eg_bc_le)egBC.SetModeExp(); else egBC.SetModeLin();

    // SR envelope
    if (trig_eg_sr_ena != -1) {
        if (data.trig[trig_eg_sr_ena] != srEGTrig) {
            srEGTrig = data.trig[trig_eg_sr_ena];
            if (!srEGTrig) egSR.Trigger();
        }
    } else {
        if (eg_sr_ena != srEGTrig) {
            srEGTrig = eg_sr_ena;
            if (srEGTrig) egSR.Trigger();
        }
    }
    if (trig_eg_sr_loop != -1) {
        egSR.SetLoop(data.trig[trig_eg_sr_loop] != 1);
    } else {
        egSR.SetLoop(eg_sr_loop);
    }

    if (cv_eg_sr_amount != -1) {
        fEGSRAmount = -0.5f * data.cv[cv_eg_sr_amount];
    } else {
        fEGSRAmount = eg_sr_amount / 4095.f * -0.5f;
    }
    if (cv_eg_sr_att != -1) {
        egSR.SetAttack(data.cv[cv_eg_sr_att] * data.cv[cv_eg_sr_att]);
    } else {
        egSR.SetAttack(eg_sr_att / 4095.f);
    }
    if (cv_eg_sr_dec != -1) {
        egSR.SetDecay(data.cv[cv_eg_sr_dec] * data.cv[cv_eg_sr_dec]);
    } else {
        egSR.SetDecay(eg_sr_dec / 4095.f);
    }
    if (eg_sr_le)egSR.SetModeExp(); else egSR.SetModeLin();
}

void ctagSoundProcessorBCSR::knowYourself() {
    // sectionCpp0
    pMapPar.emplace("level", [&](const int val) { level = val; });
    pMapCv.emplace("level", [&](const int val) { cv_level = val; });
    pMapPar.emplace("invert", [&](const int val) { invert = val; });
    pMapTrig.emplace("invert", [&](const int val) { trig_invert = val; });
    pMapPar.emplace("dry", [&](const int val) { dry = val; });
    pMapCv.emplace("dry", [&](const int val) { cv_dry = val; });
    pMapPar.emplace("bc_ena", [&](const int val) { bc_ena = val; });
    pMapTrig.emplace("bc_ena", [&](const int val) { trig_bc_ena = val; });
    pMapPar.emplace("bc_amount", [&](const int val) { bc_amount = val; });
    pMapCv.emplace("bc_amount", [&](const int val) { cv_bc_amount = val; });
    pMapPar.emplace("eg_bc_ena", [&](const int val) { eg_bc_ena = val; });
    pMapTrig.emplace("eg_bc_ena", [&](const int val) { trig_eg_bc_ena = val; });
    pMapPar.emplace("eg_bc_loop", [&](const int val) { eg_bc_loop = val; });
    pMapTrig.emplace("eg_bc_loop", [&](const int val) { trig_eg_bc_loop = val; });
    pMapPar.emplace("eg_bc_le", [&](const int val) { eg_bc_le = val; });
    pMapTrig.emplace("eg_bc_le", [&](const int val) { trig_eg_bc_le = val; });
    pMapPar.emplace("eg_bc_amount", [&](const int val) { eg_bc_amount = val; });
    pMapCv.emplace("eg_bc_amount", [&](const int val) { cv_eg_bc_amount = val; });
    pMapPar.emplace("eg_bc_att", [&](const int val) { eg_bc_att = val; });
    pMapCv.emplace("eg_bc_att", [&](const int val) { cv_eg_bc_att = val; });
    pMapPar.emplace("eg_bc_dec", [&](const int val) { eg_bc_dec = val; });
    pMapCv.emplace("eg_bc_dec", [&](const int val) { cv_eg_bc_dec = val; });
    pMapPar.emplace("sr_ena", [&](const int val) { sr_ena = val; });
    pMapTrig.emplace("sr_ena", [&](const int val) { trig_sr_ena = val; });
    pMapPar.emplace("sr_amount", [&](const int val) { sr_amount = val; });
    pMapCv.emplace("sr_amount", [&](const int val) { cv_sr_amount = val; });
    pMapPar.emplace("eg_sr_ena", [&](const int val) { eg_sr_ena = val; });
    pMapTrig.emplace("eg_sr_ena", [&](const int val) { trig_eg_sr_ena = val; });
    pMapPar.emplace("eg_sr_loop", [&](const int val) { eg_sr_loop = val; });
    pMapTrig.emplace("eg_sr_loop", [&](const int val) { trig_eg_sr_loop = val; });
    pMapPar.emplace("eg_sr_le", [&](const int val) { eg_sr_le = val; });
    pMapTrig.emplace("eg_sr_le", [&](const int val) { trig_eg_sr_le = val; });
    pMapPar.emplace("eg_sr_amount", [&](const int val) { eg_sr_amount = val; });
    pMapCv.emplace("eg_sr_amount", [&](const int val) { cv_eg_sr_amount = val; });
    pMapPar.emplace("eg_sr_att", [&](const int val) { eg_sr_att = val; });
    pMapCv.emplace("eg_sr_att", [&](const int val) { cv_eg_sr_att = val; });
    pMapPar.emplace("eg_sr_dec", [&](const int val) { eg_sr_dec = val; });
    pMapCv.emplace("eg_sr_dec", [&](const int val) { cv_eg_sr_dec = val; });
    isStereo = false;
    id = "BCSR";
    // sectionCpp0
}