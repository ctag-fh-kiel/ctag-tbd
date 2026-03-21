/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "MacroTranslator.hpp"
#include "esp_log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"
#include "helpers/ctagSampleRom.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;

/**
 * Apply a response curve to a 0-127 value.
 * All math is integer-only (no float) for ESP32 real-time safety.
 *
 * Linear:  identity (no change)
 * Log:     piecewise linear — slow start, fast end
 *            0-16  → 0-64   (×4 expansion, good for low-end detail)
 *           16-64  → 64-100 (×0.75)
 *           64-127 → 100-127 (compressed top end)
 * Exp:     value²/127 — more resolution for short times
 */
static inline int32_t applyCurve(int32_t val, MacroCurveType curve) {
    if (val <= 0) return 0;
    if (val >= 127) return 127;

    switch (curve) {
        case MacroCurveType::Log:
            // Piecewise linear: emphasises low range (great for freq/cutoff)
            if (val <= 16) {
                return val * 4;                      // 0-16 → 0-64
            } else if (val <= 64) {
                return 64 + ((val - 16) * 36) / 48;  // 16-64 → 64-100
            } else {
                return 100 + ((val - 64) * 27) / 63; // 64-127 → 100-127
            }

        case MacroCurveType::Exp:
            // Quadratic: more resolution for short decay/envelope times
            return (val * val) / 127;

        case MacroCurveType::Linear:
        default:
            return val;
    }
}


MacroTranslator::MacroTranslator() {
    soundProcessor = nullptr;
    synthDefinitionModel = nullptr;
    macroSoundDefinitionModel = nullptr;
    macroDeviceDefinitionModel = nullptr;
    bankDirty = false;

    for (int i = 0; i < 16; i++) {
        trackToMidiChannel[i] = -1;
        trackBaseCC[i] = 0;
        strcpy(trackMachineId[i], "");
        strcpy(trackSampleBankName[i], "");
        trackSampleBankIndex[i] = 0;
        definition[i] = nullptr;
        trackDirty[i] = false;

        for (int j = 0; j < 32; j++) {
            trackParameterValues[i][j] = 0;
        }
    }
};

MacroTranslator::~MacroTranslator() {
};

void MacroTranslator::SetTrackMachine(const int trackIndex, const std::string synthID) {
    if (synthID == trackMachineId[trackIndex]) {
        return;
    }

    ESP_LOGD("MacroTranslator", "Track %d machine set to %s",
        trackIndex, synthID.c_str());
    strncpy(trackMachineId[trackIndex], synthID.c_str(), sizeof(trackMachineId[trackIndex]) - 1);
    trackMachineId[trackIndex][sizeof(trackMachineId[trackIndex]) - 1] = '\0';

    SynthDefinition *synthDef = synthDefinitionModel->GetSynthDefinition(synthID);
    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Synth definition not found for id %s",
            synthID.c_str());
        return;
    }

    int idx = 0;

    TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
            trackIndex);
        return;
    }

    trackToMidiChannel[trackIndex] = trackDef->midiChannel;
    // midiChannelToTrack[trackDef->midiChannel] = trackIndex;
    trackBaseCC[trackIndex] = trackDef->baseCC;
    ESP_LOGI("MacroTranslator", "Track %d base cc is %d", trackIndex, trackBaseCC[trackIndex]);

    idx = 0;
    for(auto par : synthDef->parameters) {
        ESP_LOGI("MacroTranslator", "Processing parameter %s, type %d, cc %d",
        par->id.c_str(), par->type, par->cc);

        trackParameterValues[trackIndex][idx] = par->defaultValue;

        if (par->type == SynthParameterType_CC) {
            soundProcessor->handleMidiControlChange(
                trackDef->midiChannel,
                trackBaseCC[trackIndex] + par->cc,
                par->defaultValue
            );
        }

        idx ++;
    }

    trackDirty[trackIndex] = true;
}

void MacroTranslator::SetTrackMacroDefinition(const int trackIndex, MacroDeviceDefinition *def) {
    // ESP_LOGI("MacroTranslator", "Setting track %d macro definition 0x%08X",
    // trackIndex, (uintptr_t)def);
    if (def != nullptr) {
        ESP_LOGI("MacroTranslator", "Macro def: \"%s\" \"%s\" \"%s\"",
            def->id.c_str(), def->name.c_str(), def->synthId.c_str());
    }

    if (definition[trackIndex] != nullptr) {
        delete definition[trackIndex];
        definition[trackIndex] = nullptr;
    }

    if (def == nullptr) {
        SetTrackMachine(trackIndex, "");
        return;
    }

    SynthDefinition *synthDef =
        synthDefinitionModel->GetSynthDefinition(def->synthId);

    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Invalid synth referenceSynth definition not found for id %s",
            def->synthId.c_str());
    }

    // TrackDefinition *trackDef = synthDefinitionModel->GetTrackDefinition(trackIndex);
    // if (trackDef == nullptr) {
    //     ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
    //         trackIndex);
    //     return;
    // }

    // Document d1;
    // d1.SetObject();
    // if (def->SerializeJSONInto(d1)) {
    // ESP_LOGI("MacroTranslator", "dummy 5");
    // ESP_LOGI("MacroTranslator", "Mem 1 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
    //     heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    MacroDeviceDefinition *defcopy = def->copy();

    // ESP_LOGI("MacroTranslator", "Mem 2 freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
    //     heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
    //     heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
    //     heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    
    // if (defcopy == nullptr) {
    definition[trackIndex] = defcopy;
    // ESP_LOGI("MacroTranslator", "dummy 6");
    // } else {
    //     ESP_LOGE("MacroTranslator", "Failed to deserialize macro definition into JSON");
    //     delete defcopy;
    //     ESP_LOGI("MacroTranslator", "dummy 7");
    // }
    // } else {
    //     ESP_LOGE("MacroTranslator", "Failed to serialize macro definition into JSON");
    // }
    // ESP_LOGI("MacroTranslator", "dummy 8");


    SetTrackMachine(trackIndex, def->synthId);


    // ESP_LOGI("MacroTranslator", "dummy 9");

}

void MacroTranslator::RefreshActiveDefinitions() {
    ESP_LOGW("MacroTranslator", ">>> RefreshActiveDefinitions called");
    for (int t = 0; t < 16; t++) {
        if (definition[t] == nullptr) {
            ESP_LOGI("MacroTranslator", "  track %d: def=null, skip", t);
            continue;
        }
        std::string macroId = definition[t]->id;
        if (macroId.empty()) continue;

        MacroDeviceDefinition *freshDef =
            macroDeviceDefinitionModel->LoadMacroDeviceDefinition(macroId);
        if (freshDef == nullptr) continue;

        delete definition[t];
        definition[t] = freshDef;
        trackDirty[t] = true;

        ESP_LOGI("MacroTranslator", "Refreshed track %d def '%s' (volMult=%.2f)",
            t, macroId.c_str(), freshDef->volumeMultiplier);
    }
}

void MacroTranslator::RefreshDefinitionById(const std::string &id) {
    for (int t = 0; t < 16; t++) {
        if (definition[t] == nullptr) continue;
        if (definition[t]->id != id) continue;

        MacroDeviceDefinition *freshDef =
            macroDeviceDefinitionModel->LoadMacroDeviceDefinition(id);
        if (freshDef == nullptr) continue;

        delete definition[t];
        definition[t] = freshDef;
        trackDirty[t] = true;

        ESP_LOGI("MacroTranslator", "Refreshed track %d def '%s' (volMult=%.2f)",
            t, id.c_str(), freshDef->volumeMultiplier);
    }
}

void MacroTranslator::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    if (trackIndex < 0 || trackIndex >= 16) {
        // ESP_LOGE("MacroTranslator", "Track index out of range: %d", trackIndex);
        return;
    }

    if (parameterIndex < 0 || parameterIndex >= 32) {
        // ESP_LOGE("MacroTranslator", "Parameter index out of range: %d", parameterIndex);
        return;
    }

    // ESP_LOGI("MacroTranslator", "Track %d, Parameter %d = %d",
    //     trackIndex, parameterIndex, value);
    trackParameterValues[trackIndex][parameterIndex] = value;
    trackDirty[trackIndex] = 1;
}


void MacroTranslator::SetTrackParametersFromJSON(const std::string &parametersJSON) {
    Document d;

    d.Parse(parametersJSON.c_str());
    if (d.HasParseError()) {
        ESP_LOGE("MacroTranslator", "Failed to parse parameters JSON: %s", parametersJSON.c_str());
        return;
    }

    if (!d.IsObject()) {
        ESP_LOGE("MacroTranslator", "Parameters JSON is not an object: %s", parametersJSON.c_str());
        return;
    }

    int trackIndex = 0;
    if (d.HasMember("track")) {
        trackIndex = d["track"].GetInt();
    } else {
        ESP_LOGE("MacroTranslator", "Parameters JSON does not contain 'trackIndex': %s", parametersJSON.c_str());
        return;
    }

    if (d.HasMember("macro")) {
        std::string macro = d["macro"].GetString();
        MacroDeviceDefinition *def =
            macroDeviceDefinitionModel->LoadMacroDeviceDefinition(macro);
        ESP_LOGI("MacroTranslator", "Setting track %d macro definition to %s => 0x%08X", trackIndex, macro.c_str(), (uintptr_t)def);
        this->SetTrackMacroDefinition(trackIndex, def);
        delete def;
    }

    if (d.HasMember("machine")) {
        std::string machine = d["machine"].GetString();
        ESP_LOGI("MacroTranslator", "Setting track %d machine to: %s", trackIndex, machine.c_str());
        this->SetTrackMachine(trackIndex, machine);
    }

    if (d.HasMember("parameters")) {
        const Value& params = d["parameters"];
        if (!params.IsArray()) {
            ESP_LOGE("MacroTranslator", "'parameters' member is not an array: %s", parametersJSON.c_str());
            return;
        }

        int values[16] = {};
        for(int k=0; k<16; k++) {
            values[k] = -1;
        }
        int idx = 0;
        for (auto &v : params.GetArray()) {
            if (v.IsInt()) {
                values[idx] = v.GetInt();
                trackParameterValues[trackIndex][idx] = values[idx];
                trackDirty[trackIndex] = 1;
                // ESP_LOGD("MacroTranslator", "Set track %d parameter %d to %d",
                //     trackIndex, idx, values[idx]);
            }

            idx ++;
        }

        // MacroDeviceDefinition *def = definition[trackIndex];
        // if (def == nullptr) {
        //     ESP_LOGE("MacroTranslator", "No macro definition for track %d", trackIndex);
        //     return;
        // }
    }
}


void MacroTranslator::parseIncomingMidiMessages(const uint8_t *buf, const size_t len) {
    // if (len > 1) {
    //     ESP_LOGI("MacroTranslator",
    //         "parseIncomingMidiMessages: %02X %02X %02X %02X %02X %02X %02X %02X %02X (%d)",
    //             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], len);
    // }

    if (buf == nullptr || len < 1)  {
        return;
    }

    int left = len;
    int o = 0;

    while(left > 0) {
        uint8_t b0 = buf[o++];
        left --;

        if (b0 == 0) {
            // probably end of event stream
            return;
        }

        uint8_t inputchannel = (b0 & 0x0F);
        uint8_t cmd = (b0 & 0xF0);
        // int trackindex = midiChannelToTrack[inputchannel];

        switch(cmd) {
            case 0x80: // note off
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                soundProcessor->handleMidiNoteOff(inputchannel, note, velocity);
                break;
            }
            case 0x90: // note on
            {
                if (left < 2) return; // not enough data

                uint8_t note = buf[o++];
                uint8_t velocity = buf[o++];
                left -= 2;

                if (velocity > 0) {
                    // ESP_LOGI("MacroTranslator", "Note on, channe %d, note %d, velocity %d",
                    //     inputchannel, note, velocity);
                    soundProcessor->handleMidiNoteOn(inputchannel, note, velocity);
                } else {
                    soundProcessor->handleMidiNoteOff(inputchannel, note, velocity);
                }
                break;
            }
            case 0xA0: // aftertouch
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xB0: // control change
            {
                if (left < 2) return; // not enough data

                uint8_t control = buf[o++];
                uint8_t value = buf[o++];
                left -= 2;

                // ESP_LOGI("MacroTranslator", "got cc %d = %d on channel %d", control, value, inputchannel);

                uint8_t mapped = false;
                for(int t=0; t<16; t++) {
                    if (trackToMidiChannel[t] == inputchannel) {
                        int macrocc = (int)control - trackBaseCC[t];
                        macrocc -= 8; // input parameter CC's start at 8 too and...
                        if (macrocc >= 0 && macrocc < 24) {
                            // ESP_LOGI("MacroTranslator", "  -> track %d param %d = %d (baseCC=%d)", t, macrocc, value, trackBaseCC[t]);
                            this->SetTrackParameter(t, macrocc, value);
                            mapped = true;
                        }
                    }
                }

                if (!mapped) {
                    // ESP_LOGI("MacroTranslator", "  -> no track mapped to MIDI channel %d", inputchannel);
                    this->soundProcessor->handleMidiControlChange(inputchannel, control, value);
                }

                break;
            }
            case 0xC0: // program change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++]; // not used?
                left -= 2;
                break;
            }
            case 0xE0: // pitch bend
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                break;
            }
            case 0xF0: // system common / real time
                // TODO handle sysex, clock, start, stop, continue, ...
                switch(b0) {
                    case 0xF0: // sysex start
                        return; // just ignore and bail out for now

                    case 0xF8: // timing clock
                    case 0xFA: // start
                    case 0xFB: // continue
                    case 0xFC: // stop
                    case 0xFE: // active sensing
                    case 0xFF: // system reset
                        // ignore for now
                        break;
                }
                break; // fail for now
            default:
                // for anything else, just stop parsing.
                return;
        }
    }
}


void MacroTranslator::TranslateInput(CTAG::SP::ProcessData *pd) {
    if (pd == nullptr)
        return;
    if (soundProcessor == nullptr)
        return;

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            // First change machines if needed.
            float volMult = (definition[t] != nullptr) ? definition[t]->volumeMultiplier : 1.0f;
            soundProcessor->setTrackMachine(t, trackMachineId[t], volMult);
        }
    }

    parseIncomingMidiMessages(pd->midi_bytes, pd->midi_bytes_length);

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            trackDirty[t] = false;

            // TODO: copy mapping instead.
            MacroDeviceDefinition *def = definition[t];

            if (def != nullptr) {
                int idx = 0;
                for(auto om : def->outputMappings) {
                    int32_t finalvalue = om.startValue;

                    for(auto src : om.sources) {
                        int val = trackParameterValues[t][src.parameterIndex];
                        val = applyCurve(val, src.curve);
                        if (src.divider > 0) {
                            finalvalue += (val * src.multiplier) / src.divider;
                        } else {
                            finalvalue += val * src.multiplier;
                        }
                    }
                    
                    // TODO: support NRPM
                    
                    if (finalvalue < 0) finalvalue = 0;
                    if (finalvalue > 127) finalvalue = 127;
                    
                    int midichannel = trackToMidiChannel[t];
                    if (om.ctrl  != -1) {
                        outputValues[t][idx] = finalvalue;
                        int finalcc = om.ctrl + trackBaseCC[t];
                        soundProcessor->handleMidiControlChange(midichannel, finalcc, finalvalue);
                    }
                    idx ++;
                }
            }
        }
    }

    if (bankDirty) {
        bankDirty = false;
        for(int t=0; t<16; t++) {
            // resolve bank id from bank names
            if (trackSampleBankName[t][0] != '\0') {
                soundProcessor->setTrackBank(t, trackSampleBankIndex[t]);
            }
        }
    }
}


void MacroTranslator::SerializeStateJSON(std::string *output) {
    Document d;

    d.SetObject();

    SerializeStateInto(d);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGW("MacroTranslator", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

bool MacroTranslator::SerializeStateInto(rapidjson::Document &doc) {
    Value tracksarray(kArrayType);
    doc.AddMember("tracks", tracksarray, doc.GetAllocator());
    for(int ti =0;ti<16;ti++) {
        Value trackjson(kObjectType);
        trackjson.AddMember("index", ti, doc.GetAllocator());
        trackjson.AddMember("machine", Value(trackMachineId[ti], doc.GetAllocator()), doc.GetAllocator());
        if (definition[ti] != nullptr) {
            trackjson.AddMember("macro", Value(definition[ti]->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
        } else {
            trackjson.AddMember("macro", "", doc.GetAllocator());
        }
        // trackjson.AddMember("preset", "", doc.GetAllocator());
        doc["tracks"].PushBack(trackjson, doc.GetAllocator());
    }
    return false;
}

// void MacroTranslator::SetTrackSampleBank(const int trackIndex, const std::string bankName) {
//     if (bankName == trackSampleBankName[trackIndex]) {
//         return;
//     }

//     CTAG::SP::HELPERS::ctagSampleRom srom;
//     int bankindex = srom.GetBankIndexFromBankName(bankName);

//     ESP_LOGI("MacroTranslator", "Track %d sample bank set to %s (index %d)",
//         trackIndex, bankName.c_str(), bankindex);

//     trackSampleBankName[trackIndex] = bankName;
//     trackSampleBankIndex[trackIndex] = bankindex;

//     bankDirty = true;
// }

