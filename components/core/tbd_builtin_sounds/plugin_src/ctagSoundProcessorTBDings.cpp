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

#include <tbd/sounds/SoundProcessorTBDings.hpp>
#include <iostream>
#include <cmath>
#include "stmlib/stmlib.h"
#include <tbd/heaps.hpp>

namespace heaps = tbd::heaps;
using namespace tbd::sounds;


void SoundProcessorTBDings::Init(std::size_t blockSize, void *blockPtr) {
    /* this doesn't fit in the available block
    assert(blockSize >= 32768 * sizeof(uint16_t));
    reverb_buffer = (uint16_t *) blockPtr;
     */

    reverb_buffer = (uint16_t *) heaps::malloc(32768 * sizeof(uint16_t), TBD_HEAPS_SPIRAM);
    assert(reverb_buffer != nullptr);

    strummer.Init(0.01f, 44100.0f / bufSz);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    //memset(&patch, 0, sizeof(patch));
    //memset(&performance_state, 0, sizeof(performance_state));

    paramAD.SetSampleRate(44100.f / 32.f);
    paramAD.SetModeLin();
}

void SoundProcessorTBDings::Process(const sound_processor::ProcessData&data) {
    updateParams(data);

    bool isEaster = easter;
    if (trig_easter != -1) {
        isEaster = data.trig[trig_easter] != 1;
    }

    if (isEaster)
        string_synth.set_fx((rings::FxType) resonatorModel);
    else
        part.set_model(resonatorModel);

    // generate buffers
    float in[bufSz], out[bufSz], aux[bufSz];
    for (int i = 0; i < bufSz; i++) {
        in[i] = data.buf[i * 2];
    }
    if (isEaster) {
        strummer.Process(NULL, bufSz, &performance_state);
        string_synth.Process(performance_state, patch, in, out, aux, bufSz);
    } else {
        strummer.Process(in, bufSz, &performance_state);
        part.Process(performance_state, patch, in, out, aux, bufSz);
    }

    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] = out[i];
        data.buf[i * 2 + 1] = aux[i];
    }
}


void SoundProcessorTBDings::updateParams(const sound_processor::ProcessData&data) {
    if (cv_reson_model != -1) {
        resonatorModel = (rings::ResonatorModel) ((int) (floorf(data.cv[cv_reson_model] * 7.f)) % 6);
    } else {
        resonatorModel = (rings::ResonatorModel) ((reson_model) % 6);
    }
    if(resonatorModel>5) resonatorModel = rings::ResonatorModel(5);
    if(resonatorModel<0) resonatorModel = rings::ResonatorModel(0);


    // ad envelope
    float fAttack = eg_attack / 4095.f * 5.f;
    if (cv_eg_attack != -1) {
        fAttack = fabsf(data.cv[cv_eg_attack]) * 5.f;
    }
    CONSTRAIN(fAttack, 0.f, 5.f)
    paramAD.SetAttack(fAttack);

    float fDecay = eg_decay / 4095.f * 5.f;
    if (cv_eg_decay != -1) {
        fDecay = fabsf(data.cv[cv_eg_decay]) * 5.f;
    }
    CONSTRAIN(fDecay, 0.f, 5.f)
    paramAD.SetDecay(fDecay);

    bool triggerAD = eg_trigger;
    if (trig_eg_trigger != -1) {
        triggerAD = data.trig[trig_eg_trigger] != 1;
    }
    if (triggerAD && !eg_pre_trigger) {
        paramAD.Trigger();
    }
    eg_pre_trigger = triggerAD;

    bool loopAD = eg_loop;
    if (trig_eg_loop != -1) {
        loopAD = data.trig[trig_eg_loop] != 1;
    }
    paramAD.SetLoop(loopAD);

    // modulation
    float fAD = paramAD.Process();
    patch.brightness = brightness / 4095.f;
    if (cv_mod_brightness != -1) {
        patch.brightness += data.cv[cv_mod_brightness] * mod_brightness / 4095.f;
    } else {
        patch.brightness += fAD * mod_brightness / 4095.f;
    }
    CONSTRAIN(patch.brightness, 0.f, 1.f);
    patch.damping = damping / 4095.f;
    if (cv_mod_damping != -1) {
        patch.damping += data.cv[cv_mod_damping] * mod_damping / 4095.f;
    } else {
        patch.damping += fAD * mod_damping / 4095.f;
    }
    CONSTRAIN(patch.damping, 0.f, 1.f);
    patch.position = position / 4095.f;
    if (cv_mod_position != -1) {
        patch.position += data.cv[cv_mod_position] * mod_position / 4095.f;
    } else {
        patch.position += fAD * mod_position / 4095.f;
    }
    CONSTRAIN(patch.position, 0.f, 1.f);
    patch.structure = structure / 4095.f;
    if (cv_mod_structure != -1) {
        patch.structure += data.cv[cv_mod_structure] * mod_structure / 4095.f;
    } else {
        patch.structure += fAD * mod_structure / 4095.f;
    }
    CONSTRAIN(patch.structure, 0.f, 1.f);

    performance_state.note = frequency / 4032.f * 96.f;
    performance_state.internal_note = !note_int;
    if (cv_frequency != -1) {
        performance_state.note += data.cv[cv_frequency] * 5.f * 12.f;
    }
    CONSTRAIN(performance_state.note, 0.f, 96.f);

    if (cv_mod_frequency != -1) {
        performance_state.fm = data.cv[cv_mod_frequency] * mod_frequency / 4095.f * 48.f;
    } else {
        performance_state.fm = fAD * mod_frequency / 4095.f * 48.f;
    }
    CONSTRAIN(performance_state.fm, -48.f, 48.f);

    performance_state.tonic = 12.f + frequency / 4032.f * 60;
    CONSTRAIN(performance_state.tonic, 0.f, 60.f);

    performance_state.internal_exciter = ex_int;
    performance_state.internal_strum = !strum_int;

    if (trig_strum_int != -1) {
        if (!strum) strum = data.trig[trig_strum_int] != 1;
    } else {
        if (!strum) strum = 1;
    }
    performance_state.strum = strum && !lastStrum;
    lastStrum = strum;
    strum = false;

    performance_state.chord = chords;//(int) (patch.structure * (rings::kNumChords - 1));
    if (cv_chords != -1) {
        performance_state.chord = (int) floorf(data.cv[cv_chords] * 11.f);
    }
    CONSTRAIN(performance_state.chord, 0, 10);

    int poly = 1 << polyphony;
    CONSTRAIN(poly, 1, 4)
    if (poly != part.polyphony()) part.set_polyphony(poly);
}

SoundProcessorTBDings::~SoundProcessorTBDings() {
    heaps::free(reverb_buffer);
}
