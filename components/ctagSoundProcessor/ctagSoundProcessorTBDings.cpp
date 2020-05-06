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

#include "ctagSoundProcessorTBDings.hpp"
#include <iostream>
#include "esp_system.h"

using namespace CTAG::SP;

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

ctagSoundProcessorTBDings::ctagSoundProcessorTBDings() {
    setIsStereo();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);

    //ESP_LOGE("Rings", "Free mem %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    reverb_buffer = (uint16_t *) heap_caps_malloc(32768 * sizeof(uint16_t),
                                                  MALLOC_CAP_SPIRAM);//MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (reverb_buffer == NULL) {
        ESP_LOGE("Rings", "Could not allocate shared buffer!");
    }

    strummer.Init(0.01, 44100.0 / bufSz);
    part.Init(reverb_buffer);
    string_synth.Init(reverb_buffer);

    memset(&patch, 0, sizeof(patch));
    memset(&performance_state, 0, sizeof(performance_state));
}

void IRAM_ATTR ctagSoundProcessorTBDings::Process(const ProcessData &data) {
    updateParams(data);

    if (easter)
        string_synth.set_fx((rings::FxType) resonatorModel);
    else
        part.set_model(resonatorModel);

    // generate buffers
    float in[bufSz], out[bufSz], aux[bufSz];
    for (int i = 0; i < bufSz; i++) {
        in[i] = data.buf[i * 2];
    }
    if (easter) {
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


void ctagSoundProcessorTBDings::updateParams(const ProcessData &data) {
    resonatorModel = (rings::ResonatorModel) ((reson_model) % 6);

    patch.brightness = brightness / 4095.f;
    if (cv_mod_brightness != -1) {
        patch.brightness += data.cv[cv_mod_brightness] * mod_brightness / 4095.f;
        CONSTRAIN(patch.brightness, 0.f, 1.f);
    }
    patch.damping = damping / 4095.f;
    if (cv_mod_damping != -1) {
        patch.damping += data.cv[cv_mod_damping] * mod_damping / 4095.f;
        CONSTRAIN(patch.damping, 0.f, 1.f);
    }
    patch.position = position / 4095.f;
    if (cv_mod_position != -1) {
        patch.position += data.cv[cv_mod_position] * mod_position / 4095.f;
        CONSTRAIN(patch.position, 0.f, 1.f);
    }
    patch.structure = structure / 4095.f;
    if (cv_mod_structure != -1) {
        patch.structure += data.cv[cv_mod_structure] * mod_structure / 4095.f;
        CONSTRAIN(patch.structure, 0.f, 1.f);
    }

    performance_state.note = frequency / 4032.f * 96.f;
    performance_state.internal_note = !note_int;
    if (cv_frequency != -1) {
        performance_state.note += data.cv[cv_frequency] * 5.f * 12.f;
    }

    if (cv_mod_frequency != -1) {
        performance_state.fm = data.cv[cv_mod_frequency] * mod_frequency / 4095.f * 48.f;
        CONSTRAIN(performance_state.fm, -48.f, 48.f);
    } else {
        performance_state.fm = 0.f;
    }

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

    int poly = 1 << polyphony;
    if (poly != part.polyphony()) part.set_polyphony(poly);

}

ctagSoundProcessorTBDings::~ctagSoundProcessorTBDings() {
    heap_caps_free(reverb_buffer);
}

const char *ctagSoundProcessorTBDings::GetCStrID() const {
    return id.c_str();
}


void ctagSoundProcessorTBDings::setParamValueInternal(const string &id, const string &key, const int val) {
// autogenerated code here
// sectionCpp0
    if (id.compare("reson_model") == 0) {
        if (key.compare("current") == 0) {
            reson_model = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_reson_model = val;
        }
        return;
    }
    if (id.compare("frequency") == 0) {
        if (key.compare("current") == 0) {
            frequency = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_frequency = val;
        }
        return;
    }
    if (id.compare("polyphony") == 0) {
        if (key.compare("current") == 0) {
            polyphony = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_polyphony = val;
        }
        return;
    }
    if (id.compare("structure") == 0) {
        if (key.compare("current") == 0) {
            structure = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_structure = val;
        }
        return;
    }
    if (id.compare("brightness") == 0) {
        if (key.compare("current") == 0) {
            brightness = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_brightness = val;
        }
        return;
    }
    if (id.compare("damping") == 0) {
        if (key.compare("current") == 0) {
            damping = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_damping = val;
        }
        return;
    }
    if (id.compare("position") == 0) {
        if (key.compare("current") == 0) {
            position = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_position = val;
        }
        return;
    }
    if (id.compare("chords") == 0) {
        if (key.compare("current") == 0) {
            chords = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_chords = val;
        }
        return;
    }
    if (id.compare("easter") == 0) {
        if (key.compare("current") == 0) {
            easter = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_easter = val;
        }
        return;
    }
    if (id.compare("ex_int") == 0) {
        if (key.compare("current") == 0) {
            ex_int = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_ex_int = val;
        }
        return;
    }
    if (id.compare("strum_int") == 0) {
        if (key.compare("current") == 0) {
            strum_int = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_strum_int = val;
        }
        return;
    }
    if (id.compare("note_int") == 0) {
        if (key.compare("current") == 0) {
            note_int = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_note_int = val;
        }
        return;
    }
    if (id.compare("mod_brightness") == 0) {
        if (key.compare("current") == 0) {
            mod_brightness = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_mod_brightness = val;
        }
        return;
    }
    if (id.compare("mod_frequency") == 0) {
        if (key.compare("current") == 0) {
            mod_frequency = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_mod_frequency = val;
        }
        return;
    }
    if (id.compare("mod_damping") == 0) {
        if (key.compare("current") == 0) {
            mod_damping = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_mod_damping = val;
        }
        return;
    }
    if (id.compare("mod_structure") == 0) {
        if (key.compare("current") == 0) {
            mod_structure = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_mod_structure = val;
        }
        return;
    }
    if (id.compare("mod_position") == 0) {
        if (key.compare("current") == 0) {
            mod_position = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_mod_position = val;
        }
        return;
    }
// sectionCpp0







}

void ctagSoundProcessorTBDings::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
    reson_model = model->GetParamValue("reson_model", "current");
    cv_reson_model = model->GetParamValue("reson_model", "cv");
    frequency = model->GetParamValue("frequency", "current");
    cv_frequency = model->GetParamValue("frequency", "cv");
    polyphony = model->GetParamValue("polyphony", "current");
    cv_polyphony = model->GetParamValue("polyphony", "cv");
    structure = model->GetParamValue("structure", "current");
    cv_structure = model->GetParamValue("structure", "cv");
    brightness = model->GetParamValue("brightness", "current");
    cv_brightness = model->GetParamValue("brightness", "cv");
    damping = model->GetParamValue("damping", "current");
    cv_damping = model->GetParamValue("damping", "cv");
    position = model->GetParamValue("position", "current");
    cv_position = model->GetParamValue("position", "cv");
    chords = model->GetParamValue("chords", "current");
    cv_chords = model->GetParamValue("chords", "cv");
    easter = model->GetParamValue("easter", "current");
    trig_easter = model->GetParamValue("easter", "trig");
    ex_int = model->GetParamValue("ex_int", "current");
    trig_ex_int = model->GetParamValue("ex_int", "trig");
    strum_int = model->GetParamValue("strum_int", "current");
    trig_strum_int = model->GetParamValue("strum_int", "trig");
    note_int = model->GetParamValue("note_int", "current");
    trig_note_int = model->GetParamValue("note_int", "trig");
    mod_brightness = model->GetParamValue("mod_brightness", "current");
    cv_mod_brightness = model->GetParamValue("mod_brightness", "cv");
    mod_frequency = model->GetParamValue("mod_frequency", "current");
    cv_mod_frequency = model->GetParamValue("mod_frequency", "cv");
    mod_damping = model->GetParamValue("mod_damping", "current");
    cv_mod_damping = model->GetParamValue("mod_damping", "cv");
    mod_structure = model->GetParamValue("mod_structure", "current");
    cv_mod_structure = model->GetParamValue("mod_structure", "cv");
    mod_position = model->GetParamValue("mod_position", "current");
    cv_mod_position = model->GetParamValue("mod_position", "cv");
// sectionCpp1

}

