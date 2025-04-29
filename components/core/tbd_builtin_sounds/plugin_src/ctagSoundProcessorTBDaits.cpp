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

#include <tbd/sounds/SoundProcessorTBDaits.hpp>
#include <iostream>
#include <tbd/sound_utils/ctagFastMath.hpp>

using namespace tbd::sounds;

void SoundProcessorTBDaits::Init(std::size_t blockSize, void *blockPtr) {
    assert(blockSize >= 16384);
    shared_buffer = (char *) blockPtr;

    stmlib::BufferAllocator allocator(shared_buffer, 16384);
    voice.Init(&allocator);

    patch.engine = 0;
    patch.lpg_colour = 0.5f;
    patch.decay = 0.5f;
}

void SoundProcessorTBDaits::Process(const audio::ProcessData&data) {
    patch.note = frequency / 4095.f * 96.f;

    if (cv_lpg_decay != -1) {
        patch.decay = sound_utils::fastfabs(data.cv[cv_lpg_decay]);
    } else {
        patch.decay = lpg_decay / 4095.f;
    }

    patch.harmonics = harmonics / 4095.f;

    if (cv_lpg_color != -1) {
        patch.lpg_colour = sound_utils::fastfabs(data.cv[cv_lpg_color]);
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
        modulations.engine = sound_utils::fastfabs(data.cv[cv_smodel]);
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
        modulations.level = sound_utils::fastfabs(data.cv[cv_level]);
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

SoundProcessorTBDaits::~SoundProcessorTBDaits() {
}
