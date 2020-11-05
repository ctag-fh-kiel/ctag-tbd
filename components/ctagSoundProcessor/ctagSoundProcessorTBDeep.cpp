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

#include "ctagSoundProcessorTBDeep.hpp"
#include <iostream>
#include "helpers/ctagFastMath.hpp"
#include "esp_system.h"
#include <cmath>
#include "esp_log.h"
#include "esp_heap_caps.h"

// part of the port is adapted from VCV Rack Audible Instruments, (C) Andrew Belt
using namespace CTAG::SP;

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

static const float kRootScaled[3] = {
        0.125f,
        2.0f,
        130.81f
};

static const tides2::Ratio kRatios[20] = {
        {0.0625f,    16},
        {0.125f,     8},
        {0.1666666f, 6},
        {0.25f,      4},
        {0.3333333f, 3},
        {0.5f,       2},
        {0.6666666f, 3},
        {0.75f,      4},
        {0.8f,       5},
        {1,          1},
        {1,          1},
        {1.25f,      4},
        {1.3333333f, 3},
        {1.5f,       2},
        {2.0f,       1},
        {3.0f,       1},
        {4.0f,       1},
        {6.0f,       1},
        {8.0f,       1},
        {16.0f,      1},
};

ctagSoundProcessorTBDeep::ctagSoundProcessorTBDeep() {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    poly_slope_generator.Init();
    ratio_index_quantizer.Init();
    output_mode = tides2::OUTPUT_MODE_GATES;
    ramp_mode = tides2::RAMP_MODE_LOOPING;
    ramp_extractor.Init(44100.f, 40.f / 44100.f);

    loudAD.SetSampleRate(44100.f);
    loudAD.SetModeExp();
    paramAD.SetSampleRate(44100.f / 32.f);
    paramAD.SetModeLin();
}

void ctagSoundProcessorTBDeep::Process(const ProcessData &data) {
    // ad envelope
    float fAttack = eg_attack / 4095.f * 5.f;
    if (cv_eg_attack != -1) {
        fAttack = fabsf(data.cv[cv_eg_attack]) * 5.f;
    }
    loudAD.SetAttack(fAttack);
    paramAD.SetAttack(fAttack);

    float fDecay = eg_decay / 4095.f * 5.f;
    if (cv_eg_decay != -1) {
        fDecay = fabsf(data.cv[cv_eg_decay]) * 5.f;
    }
    loudAD.SetDecay(fDecay);
    paramAD.SetDecay(fDecay);

    bool triggerAD = eg_trigger;
    if (trig_eg_trigger != -1) {
        triggerAD = data.trig[trig_eg_trigger] != 1;
    }
    if (triggerAD && !eg_pre_trigger) {
        loudAD.Trigger();
        paramAD.Trigger();
    }

    eg_pre_trigger = triggerAD;
    bool loopAD = eg_loop;
    if (trig_eg_loop != -1) {
        loopAD = data.trig[trig_eg_loop] != 1;
    }
    loudAD.SetLoop(loopAD);
    paramAD.SetLoop(loopAD);

    // sheep setup
    output_mode = (tides2::OutputMode) 3; // only mode 3 (tides2::OutputMode)(mode % 4);
    ramp_mode = (tides2::RampMode) 1; // only mode 1

    // Input gates
    for (int i = 0; i < bufSz; i++) {
        if (trig_trigger != -1) {
            trig_flags[i] = stmlib::ExtractGateFlags(previous_trig_flag, data.trig[trig_trigger] != 1);
        } else {
            trig_flags[i] = trigger;
        }
        previous_trig_flag = trig_flags[i];

        if (trig_clock != -1) {
            clock_flags[i] = stmlib::ExtractGateFlags(previous_clock_flag, data.trig[trig_clock] != 1);
        } else {
            clock_flags[i] = clock;
        }
        previous_clock_flag = clock_flags[i];
    }

    float note = frequency;
    if (cv_frequency != -1) {
        note += 12.f * data.cv[cv_frequency] * 5.f;
    }
    CONSTRAIN(note, -96.f, 96.f);

    float eg = paramAD.Process();
    float fm = mod_frequency / 4095.f * 12.f * 5.f;
    if (cv_mod_frequency != -1) {
        fm *= data.cv[cv_mod_frequency];
    } else {
        fm *= eg;
    }
    CONSTRAIN(fm, -96.f, 96.f);

    float transposition = note + fm;
    float ramp[tides2::kBlockSize];
    float freq;
    tides2::Range range_mode = tides2::RANGE_AUDIO; // only audio (range < 2) ? tides2::RANGE_CONTROL : tides2::RANGE_AUDIO;

    if (trig_clock != -1) {
        if (must_reset_ramp_extractor) {
            ramp_extractor.Reset();
        }

        tides2::Ratio r = ratio_index_quantizer.Lookup(kRatios, 0.5f + transposition * 0.0105f, 20);
        freq = ramp_extractor.Process(
                range_mode == tides2::RANGE_AUDIO,
                range_mode == tides2::RANGE_AUDIO && ramp_mode == tides2::RAMP_MODE_AR,
                r,
                clock_flags,
                ramp,
                tides2::kBlockSize);
        must_reset_ramp_extractor = false;
    } else {
        freq = kRootScaled[2] / 44100.f * stmlib::SemitonesToRatio(transposition);
        must_reset_ramp_extractor = true;
    }

    // Get parameters
    float fSlope = slope / 4095.f;
    if (cv_slope != -1) {
        fSlope = fabsf(data.cv[cv_slope]);
    }
    float fShape = shape / 4095.f;
    if (cv_shape != -1) {
        fShape = fabsf(data.cv[cv_shape]);
    }
    float fSmoothness = smoothness / 4095.f;
    if (cv_smoothness != -1) {
        fSmoothness = fabsf(data.cv[cv_smoothness]);
    }
    float fShift = shift / 4095.f;
    if (cv_shift != -1) {
        fShift = fabsf(data.cv[cv_shift]);
    }


    if (cv_mod_slope != -1) {
        fSlope += mod_slope / 4095.f * data.cv[cv_mod_slope];
    } else {
        fSlope += eg * mod_slope / 4095.f;
    }
    CONSTRAIN(fSlope, 0.f, 1.f);
    if (cv_mod_shape != -1) {
        fShape += mod_shape / 4095.f * data.cv[cv_mod_shape];
    } else {
        fShape += eg * mod_shape / 4095.f;
    }
    CONSTRAIN(fShape, 0.f, 1.f);
    if (cv_mod_smoothness != -1) {
        fSmoothness += mod_smoothness / 4095.f * data.cv[cv_mod_smoothness];
    } else {
        fSmoothness += eg * mod_smoothness / 4095.f;
    }
    CONSTRAIN(fSmoothness, 0.f, 1.f);
    if (cv_shift != -1) {
        fShift += mod_shift / 4095.f * data.cv[cv_mod_shift];
    } else {
        fShift += eg * mod_shift / 4095.f;
    }
    CONSTRAIN(fShift, 0.f, 1.f);
    if (output_mode != previous_output_mode) {
        poly_slope_generator.Reset();
        previous_output_mode = output_mode;
    }

    // Render generator
    poly_slope_generator.Render(
            ramp_mode,
            output_mode,
            range_mode,
            freq,
            fSlope,
            fShape,
            fSmoothness,
            fShift,
            trig_flags,
            (trig_trigger == -1) && (trig_clock != -1) ? ramp : NULL,
            out,
            tides2::kBlockSize);

    // which output to route?
    int o0 = out0;
    if (cv_out0 != -1) {
        o0 = fabs(data.cv[cv_out0] * 4.f);
        CONSTRAIN(o0, 0, 3);
    }

    int o1 = out1;
    if (cv_out1 != -1) {
        o1 = fabs(data.cv[cv_out1] * 4.f);
        CONSTRAIN(o1, 0, 3);
    }

    // loudness
    float fLoud0 = out0_level / 4095.f * 0.25f;
    if (cv_out0_level != -1) {
        fLoud0 = data.cv[cv_out0_level];
    }
    float fLoud1 = out1_level / 4095.f;
    if (cv_out1_level != -1) {
        fLoud1 = data.cv[cv_out1_level] * 0.25f;
    }
    // loudness modulation
    float fModLevel = mod_level / 4095.f;
    if (cv_mod_level != -1) {
        fModLevel = data.cv[cv_mod_level];
    }

    for (int j = 0; j < bufSz; j++) {
        float loud = fModLevel * loudAD.Process();
        if (fModLevel < 0.f) loud -= fModLevel;
        float loud0 = fLoud0 * ((1.f - fabsf(fModLevel)) + loud);
        float loud1 = fLoud1 * ((1.f - fabsf(fModLevel)) + loud);
        data.buf[j * 2] = HELPERS::fasttanh(out[j].channel[o0] * loud0);
        data.buf[j * 2 + 1] = HELPERS::fasttanh(out[j].channel[o1] * loud1);
    }
}

void ctagSoundProcessorTBDeep::knowYourself() {
// sectionCpp0
    pMapPar.emplace("trigger", [&](const int val) { trigger = val; });
    pMapTrig.emplace("trigger", [&](const int val) { trig_trigger = val; });
    pMapPar.emplace("clock", [&](const int val) { clock = val; });
    pMapTrig.emplace("clock", [&](const int val) { trig_clock = val; });
    pMapPar.emplace("out0", [&](const int val) { out0 = val; });
    pMapCv.emplace("out0", [&](const int val) { cv_out0 = val; });
    pMapPar.emplace("out0_level", [&](const int val) { out0_level = val; });
    pMapCv.emplace("out0_level", [&](const int val) { cv_out0_level = val; });
    pMapPar.emplace("out1", [&](const int val) { out1 = val; });
    pMapCv.emplace("out1", [&](const int val) { cv_out1 = val; });
    pMapPar.emplace("out1_level", [&](const int val) { out1_level = val; });
    pMapCv.emplace("out1_level", [&](const int val) { cv_out1_level = val; });
    pMapPar.emplace("frequency", [&](const int val) { frequency = val; });
    pMapCv.emplace("frequency", [&](const int val) { cv_frequency = val; });
    pMapPar.emplace("shape", [&](const int val) { shape = val; });
    pMapCv.emplace("shape", [&](const int val) { cv_shape = val; });
    pMapPar.emplace("slope", [&](const int val) { slope = val; });
    pMapCv.emplace("slope", [&](const int val) { cv_slope = val; });
    pMapPar.emplace("smoothness", [&](const int val) { smoothness = val; });
    pMapCv.emplace("smoothness", [&](const int val) { cv_smoothness = val; });
    pMapPar.emplace("shift", [&](const int val) { shift = val; });
    pMapCv.emplace("shift", [&](const int val) { cv_shift = val; });
    pMapPar.emplace("eg_trigger", [&](const int val) { eg_trigger = val; });
    pMapTrig.emplace("eg_trigger", [&](const int val) { trig_eg_trigger = val; });
    pMapPar.emplace("eg_loop", [&](const int val) { eg_loop = val; });
    pMapTrig.emplace("eg_loop", [&](const int val) { trig_eg_loop = val; });
    pMapPar.emplace("eg_attack", [&](const int val) { eg_attack = val; });
    pMapCv.emplace("eg_attack", [&](const int val) { cv_eg_attack = val; });
    pMapPar.emplace("eg_decay", [&](const int val) { eg_decay = val; });
    pMapCv.emplace("eg_decay", [&](const int val) { cv_eg_decay = val; });
    pMapPar.emplace("mod_level", [&](const int val) { mod_level = val; });
    pMapCv.emplace("mod_level", [&](const int val) { cv_mod_level = val; });
    pMapPar.emplace("mod_frequency", [&](const int val) { mod_frequency = val; });
    pMapCv.emplace("mod_frequency", [&](const int val) { cv_mod_frequency = val; });
    pMapPar.emplace("mod_shape", [&](const int val) { mod_shape = val; });
    pMapCv.emplace("mod_shape", [&](const int val) { cv_mod_shape = val; });
    pMapPar.emplace("mod_slope", [&](const int val) { mod_slope = val; });
    pMapCv.emplace("mod_slope", [&](const int val) { cv_mod_slope = val; });
    pMapPar.emplace("mod_smoothness", [&](const int val) { mod_smoothness = val; });
    pMapCv.emplace("mod_smoothness", [&](const int val) { cv_mod_smoothness = val; });
    pMapPar.emplace("mod_shift", [&](const int val) { mod_shift = val; });
    pMapCv.emplace("mod_shift", [&](const int val) { cv_mod_shift = val; });
    isStereo = true;
    id = "TBDeep";
    // sectionCpp0
}
