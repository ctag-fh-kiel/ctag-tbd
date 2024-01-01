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

#include "ctagSoundProcessorTBDaits.hpp"
#include <iostream>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "helpers/ctagFastMath.hpp"
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::SP;

void ctagSoundProcessorTBDaits::Init(std::size_t blockSize, void *blockPtr) {
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    shared_buffer = (char *) heap_caps_malloc(16384, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (shared_buffer == NULL) {
        ESP_LOGE("Plaits", "Could not allocate shared buffer!");
    }
    stmlib::BufferAllocator allocator(shared_buffer, 16384);
    voice.Init(&allocator);

    patch.engine = 0;
    patch.lpg_colour = 0.5f;
    patch.decay = 0.5f;
}

void ctagSoundProcessorTBDaits::Process(const ProcessData &data) {
    patch.note = frequency / 4095.f * 96.f;

    if (cv_lpg_decay != -1) {
        patch.decay = HELPERS::fastfabs(data.cv[cv_lpg_decay]);
    } else {
        patch.decay = lpg_decay / 4095.f;
    }

    patch.harmonics = harmonics / 4095.f;

    if (cv_lpg_color != -1) {
        patch.lpg_colour = HELPERS::fastfabs(data.cv[cv_lpg_color]);
    } else {
        patch.lpg_colour = lpg_color / 4095.f;
    }

    patch.timbre = timbre / 4095.f;
    patch.morph = morph / 4095.f;
    patch.morph_modulation_amount = mod_morph / 4095.f;
    patch.frequency_modulation_amount = mod_freq / 4095.f;
    patch.timbre_modulation_amount = mod_timbre / 4095.f;
    patch.engine = smodel;

    plaits::Modulations modulations;

    if (cv_smodel != -1) {
        modulations.engine = HELPERS::fastfabs(data.cv[cv_smodel]);
    } else {
        modulations.engine = 0.f;
    }

    if (cv_frequency != -1) {
        modulations.note = data.cv[cv_frequency] * 5.f * 12.f;
    } else {
        modulations.note = 0.f;
    }

    if (cv_mod_freq != -1) {
        modulations.frequency_patched = true;
        modulations.frequency = data.cv[cv_mod_freq] * 30.f;
    } else {
        modulations.frequency_patched = false;
        modulations.frequency = 0.f;
    }

    if (cv_harmonics != -1) {
        modulations.harmonics = data.cv[cv_harmonics];
    } else {
        modulations.harmonics = 0.f;
    }

    if (cv_mod_timbre != -1) {
        modulations.timbre = data.cv[cv_mod_timbre];
        modulations.timbre_patched = true;
    } else {
        modulations.timbre = 0.f;
        modulations.timbre_patched = false;
    }

    if (cv_mod_morph != -1) {
        modulations.morph = data.cv[cv_mod_morph];
        modulations.morph_patched = true;
    } else {
        modulations.morph = 0.f;
        modulations.morph_patched = false;
    }

    if (trig_trigger != -1) {
        modulations.trigger_patched = true;
        modulations.trigger = data.trig[trig_trigger] != 1;
    } else {
        modulations.trigger_patched = false;
        modulations.trigger = false;
    }

    float fLevel = 1.f;
    if (cv_level != -1) {
        modulations.level = HELPERS::fastfabs(data.cv[cv_level]);
        modulations.level_patched = true;
    } else {
        fLevel = level / 4095.f;
        modulations.level_patched = false;
    }

    // get frame
    plaits::Voice::Frame out[bufSz];
    voice.Render(patch, modulations, out, bufSz);

    // convert
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = out[i].out / 32768.f * fLevel;
        data.buf[i * 2 + 1] = out[i].aux / 32768.f * fLevel;
    }
}

ctagSoundProcessorTBDaits::~ctagSoundProcessorTBDaits() {
    heap_caps_free(shared_buffer);
}

void ctagSoundProcessorTBDaits::knowYourself() {
// sectionCpp0
    pMapPar.emplace("smodel", [&](const int val) { smodel = val; });
    pMapCv.emplace("smodel", [&](const int val) { cv_smodel = val; });
    pMapPar.emplace("trigger", [&](const int val) { trigger = val; });
    pMapTrig.emplace("trigger", [&](const int val) { trig_trigger = val; });
    pMapPar.emplace("frequency", [&](const int val) { frequency = val; });
    pMapCv.emplace("frequency", [&](const int val) { cv_frequency = val; });
    pMapPar.emplace("level", [&](const int val) { level = val; });
    pMapCv.emplace("level", [&](const int val) { cv_level = val; });
    pMapPar.emplace("harmonics", [&](const int val) { harmonics = val; });
    pMapCv.emplace("harmonics", [&](const int val) { cv_harmonics = val; });
    pMapPar.emplace("timbre", [&](const int val) { timbre = val; });
    pMapCv.emplace("timbre", [&](const int val) { cv_timbre = val; });
    pMapPar.emplace("morph", [&](const int val) { morph = val; });
    pMapCv.emplace("morph", [&](const int val) { cv_morph = val; });
    pMapPar.emplace("lpg_color", [&](const int val) { lpg_color = val; });
    pMapCv.emplace("lpg_color", [&](const int val) { cv_lpg_color = val; });
    pMapPar.emplace("lpg_decay", [&](const int val) { lpg_decay = val; });
    pMapCv.emplace("lpg_decay", [&](const int val) { cv_lpg_decay = val; });
    pMapPar.emplace("mod_freq", [&](const int val) { mod_freq = val; });
    pMapCv.emplace("mod_freq", [&](const int val) { cv_mod_freq = val; });
    pMapPar.emplace("mod_timbre", [&](const int val) { mod_timbre = val; });
    pMapCv.emplace("mod_timbre", [&](const int val) { cv_mod_timbre = val; });
    pMapPar.emplace("mod_morph", [&](const int val) { mod_morph = val; });
    pMapCv.emplace("mod_morph", [&](const int val) { cv_mod_morph = val; });
    isStereo = true;
    id = "TBDaits";
    // sectionCpp0
}
