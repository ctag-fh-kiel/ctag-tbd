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
#include "SPManager.hpp"
#include "esp_log.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "helpers/ctagSampleRom.hpp"
#include "EngineDefinitionDataModel.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;

/**
 * Apply a response curve to a 0..maxVal value.
 *
 * Matches the legacy 0..127-domain piecewise curve shape exactly when
 * maxVal == 127, but computes natively in the maxVal domain using 64-bit
 * integer math so there are no precision-loss plateaus on hi-res sources
 * (maxVal == 16383). All math is integer-only (no float) for ESP32
 * real-time safety.
 *
 * Linear:  identity (no change)
 * Log:     piecewise linear — slow start, fast end
 *            0..(M/8)      → 0..(M*64/127)            (×4 expansion)
 *            (M/8)..(M/2)  → (M*64/127)..(M*100/127)  (×0.75)
 *            (M/2)..M      → (M*100/127)..M           (compressed top)
 * Exp:     val² / maxVal — more resolution for short times
 *
 * Why 64-bit intermediates: the old version normalised `val` into a 0..127
 * domain first (`n = val * 127 / maxVal`). For `maxVal == 16383`, that
 * truncated any val below ~129 to n=0 and created maxVal/127-wide plateaus
 * of identical output across consecutive input values. Knobs with curves
 * on hi-res sources felt "dead" at slow turn speeds. See the AnaKick A FM
 * / FM Kick Default debugging session (2026-04-24) and hi-res-and-nrpn.md
 * § "Debugging unresponsive knobs".
 */
static inline int32_t applyCurve(int32_t val, int32_t maxVal, MacroCurveType curve) {
    if (curve == MacroCurveType::Linear) return val;
    if (val <= 0) return 0;
    if (val >= maxVal) return maxVal;

    int64_t v = val;
    int64_t M = maxVal;
    int64_t out;

    switch (curve) {
        case MacroCurveType::Log:
            // Piecewise linear — breakpoints at 16/127 (~12.6%) and 64/127
            // (~50%) of the input range, matching the legacy 127-domain
            // shape. Comparisons use v*127 vs M*k to stay in integer math
            // without dividing (no truncation loss on hi-res).
            if (v * 127 <= M * 16) {
                // Segment 1 (slope ×4): y = v * 4
                out = v * 4;
            } else if (v * 127 <= M * 64) {
                // Segment 2: y = M*64/127 + (v*127 - M*16) * 36 / (48 * 127)
                out = (M * 64) / 127 + ((v * 127 - M * 16) * 36) / (48 * 127);
            } else {
                // Segment 3: y = M*100/127 + (v*127 - M*64) * 27 / (63 * 127)
                out = (M * 100) / 127 + ((v * 127 - M * 64) * 27) / (63 * 127);
            }
            break;

        case MacroCurveType::Exp:
            // Quadratic: (v*127/M)² / 127, scaled back to maxVal domain.
            // Algebraic simplification: (v*127/M)² / 127 * M/127 = v² / M.
            out = (v * v) / M;
            break;

        default:
            out = v;
            break;
    }

    if (out < 0) return 0;
    if (out > maxVal) return maxVal;
    return (int32_t)out;
}

void MacroTranslator::Init() {
    soundProcessor = nullptr;
    bankDirty = false;
    definitions = (class MacroDeviceDefinition *)heap_caps_malloc(sizeof(MacroDeviceDefinition) * 16, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);

    for (int i = 0; i < 16; i++) {
        trackToMidiChannel[i] = -1;
        trackBaseCC[i] = 0;
        strcpy(trackMachineId[i], "");
        strcpy(trackSampleBankName[i], "");
        trackSampleBankIndex[i] = 0;
        trackDirty[i] = false;

        nrpm_number_lsb[i] = 0;
        nrpm_number_msb[i] = 0;
        nrpm_value_lsb[i] = 0;
        nrpm_value_msb[i] = 0;

        MacroDeviceDefinitionUtils::MacroDeviceDefinition_Reset(&definitions[i]);

        for (int j = 0; j < 24; j++) {
            trackParameterValues[i][j] = 0;
        }
    }
}

void MacroTranslator::SetTrackMachine(const int trackIndex, const std::string synthID, float volumeMultiplier) {
    if (synthID == trackMachineId[trackIndex]) {
        return;
    }

    ESP_LOGD("MacroTranslator", "Track %d machine set to %s",
        trackIndex, synthID.c_str());
    strncpy(trackMachineId[trackIndex], synthID.c_str(), sizeof(trackMachineId[trackIndex]) - 1);
    trackMachineId[trackIndex][sizeof(trackMachineId[trackIndex]) - 1] = '\0';

    struct SharedEngineDefinition *synthDef = EngineDefinitionDataModel::instance()->GetSynthDefinition(synthID);
    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Synth definition not found for id %s",
            synthID.c_str());
        return;
    }

    int idx = 0;

    struct TrackDefinition *trackDef = EngineDefinitionDataModel::instance()->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Track definition not found for track index %d",
            trackIndex);
        return;
    }

    trackToMidiChannel[trackIndex] = trackDef->midiChannel;
    // midiChannelToTrack[trackDef->midiChannel] = trackIndex;
    trackBaseCC[trackIndex] = trackDef->baseCC;
    ESP_LOGI("MacroTranslator", "Track %d base cc is %d", trackIndex, trackBaseCC[trackIndex]);

    for(idx=0; idx<MaxEngineDefinitionParameters; idx++) {
        struct SharedEngineDefinitionParameter &par = synthDef->parameters[idx];
        if (par.id[0] == '\0') {
            break;
        }

        ESP_LOGI("MacroTranslator", "Processing parameter %s, type %d, cc %d",
        par.id, par.type, par.relCC);

        trackParameterValues[trackIndex][idx] = par.defaultValue;

        if (par.type == EngineParameterType_CC) {
            soundProcessor->handleMidiControlChange(
                trackDef->midiChannel,
                trackBaseCC[trackIndex] + par.relCC,
                par.defaultValue
            );
        }
        if (par.type == EngineParameterType_NRPM) {
            soundProcessor->handleMidiControlChangeNRPM(
                trackDef->midiChannel,
                trackBaseCC[trackIndex] + par.relCC,
                par.defaultValue
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
        ESP_LOGI("MacroTranslator", "Macro def: \"%s\" \"%s\" \"%s\" %1.1fx",
            def->id, def->name, def->synthId, def->volumeMultiplier);
    }

    if (def == nullptr) {
        SetTrackMachine(trackIndex, "", 1.0f);
        return;
    }

    struct SharedEngineDefinition *synthDef =
        EngineDefinitionDataModel::instance()->GetSynthDefinition(def->synthId);

    if (synthDef == nullptr) {
        ESP_LOGE("MacroTranslator", "Invalid synth referenceSynth definition not found for id %s",
            def->synthId);
    }


    MacroDeviceDefinitionUtils::MacroDeviceDefinition_CopyInto(def, &definitions[trackIndex]);
    SetTrackMachine(trackIndex, def->synthId, def->volumeMultiplier);
}

void MacroTranslator::RefreshActiveDefinitions() {
    ESP_LOGW("MacroTranslator", ">>> RefreshActiveDefinitions called");
    for (int t = 0; t < 16; t++) {
        if (definitions[t].id[0] == '\0') {
            ESP_LOGI("MacroTranslator", "  track %d: def=null, skip", t);
            continue;
        }
        std::string macroId = definitions[t].id;
        if (macroId.empty()) continue;

        MacroDeviceDefinition *freshDef =
            MacroDeviceDefinitionDataModel::instance().GetMacroDeviceDefinition(macroId.c_str());
        if (freshDef == nullptr) continue;

        // delete definition[t];
        definitions[t] = *freshDef;
        // trackDirty[t] = true;  // NOT here — bulk reload is too heavy, use RefreshDefinitionById() for live edit

        ESP_LOGI("MacroTranslator", "Refreshed track %d def '%s' (volMult=%.2f)",
            t, macroId.c_str(), freshDef->volumeMultiplier);
    }
}

void MacroTranslator::RefreshDefinitionById(const std::string &id) {
    for (int t = 0; t < 16; t++) {
        if (definitions[t].id[0] == '\0') continue;
        if (strcmp(definitions[t].id, id.c_str()) != 0) continue;

        MacroDeviceDefinition *freshDef =
            MacroDeviceDefinitionDataModel::instance().GetMacroDeviceDefinition(id.c_str());
        if (freshDef == nullptr) continue;

        // Check if machine changed — if so, do full machine init (resets params to defaults)
        bool machineChanged = (strcmp(freshDef->synthId, trackMachineId[t]) != 0);

        definitions[t] = *freshDef;

        // Invalidate output value cache — forces all mappings to re-send to DSP
        memset(outputValues[t], 0xFF, sizeof(outputValues[t]));

        if (machineChanged && freshDef->synthId[0] != '\0') {
            ESP_LOGI("MacroTranslator", "Live-edit: track %d machine changed to '%s', full init",
                t, freshDef->synthId);
            // SetTrackMachine updates trackMachineId, resets param values to synth defaults,
            // and sets trackDirty — needed when switching machines
            SetTrackMachine(t, std::string(freshDef->synthId), freshDef->volumeMultiplier);
        } else {
            // Same machine — push the new volMult to the rack mixer (lightweight,
            // no state reset), update trackMachineId, mark dirty so mappings
            // re-fan-out to DSP.
            //
            // CRITICAL: SoundProcessorManager::SetTrackVolumeMultiplier MUST NOT
            // take processMutex — the caller (RefreshSingleMacro) already holds
            // it. Non-recursive FreeRTOS mutex would deadlock the audio task.
            CTAG::AUDIO::SoundProcessorManager::SetTrackVolumeMultiplier(t, freshDef->volumeMultiplier);
            snprintf(trackMachineId[t], sizeof(trackMachineId[t]), "%s", freshDef->synthId);
            trackDirty[t] = true;
        }

        ESP_LOGI("MacroTranslator", "Live-edit: refreshed track %d def '%s' (volMult=%.2f, machineChanged=%d)",
            t, id.c_str(), freshDef->volumeMultiplier, machineChanged);
    }
}

void MacroTranslator::SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value) {
    if (trackIndex < 0 || trackIndex >= 16) {
        // ESP_LOGE("MacroTranslator", "Track index out of range: %d", trackIndex);
        return;
    }

    // Storage is trackParameterValues[16][24] — accept idx 0..23.
    if (parameterIndex < 0 || parameterIndex >= 24) {
        // ESP_LOGE("MacroTranslator", "Parameter index out of range: %d", parameterIndex);
        return;
    }

    // ESP_LOGI("MacroTranslator", "Track %d, Parameter %d = %d",     trackIndex, parameterIndex, value);
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
            MacroDeviceDefinitionDataModel::instance().GetMacroDeviceDefinition(macro.c_str());
        ESP_LOGI("MacroTranslator", "Setting track %d macro definition to %s => 0x%08X", trackIndex, macro.c_str(), (uintptr_t)def);
        this->SetTrackMacroDefinition(trackIndex, def);
        delete def;
    }
    else if (d.HasMember("machine")) {
        std::string machine = d["machine"].GetString();
        ESP_LOGI("MacroTranslator", "Setting track %d machine to: %s", trackIndex, machine.c_str());
        this->SetTrackMachine(trackIndex, machine, 1.0f);
    }

    if (d.HasMember("parameters")) {
        const Value& params = d["parameters"];
        if (!params.IsArray()) {
            ESP_LOGE("MacroTranslator", "'parameters' member is not an array: %s", parametersJSON.c_str());
            return;
        }

        // trackParameterValues is now [16][24] — idx 0..23 are in bounds. JSON entries
        // beyond 24 are truncated.
        int values[24] = {};
        for(int k=0; k<24; k++) {
            values[k] = -1;
        }
        int idx = 0;
        for (auto &v : params.GetArray()) {
            if (idx >= 24) break;
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

                // TODO: support NRPM
                if (control == 98) {
                    // NRPM number LSB

                    nrpm_number_lsb[inputchannel] = value;

                } else if(control == 99) {
                    // NRPM number MSB
                    nrpm_number_msb[inputchannel] = value;

                } else if(control == 38) {
                    // NRPM value LSB
                    nrpm_value_lsb[inputchannel] = value;

                } else if(control == 6) {
                    // NRPM value MSB
                    nrpm_value_msb[inputchannel] = value;

                    int16_t number2 = (nrpm_number_msb[inputchannel] << 7) + nrpm_number_lsb[inputchannel];
                    int16_t value2 = (nrpm_value_msb[inputchannel] << 7) + nrpm_value_lsb[inputchannel];

                    // ESP_LOGI("MacroTranslator", "Received NRPM on channel %d: number %d|%d (%d), value %d|%d (%d)",
                    //     inputchannel,
                    //     nrpm_number_msb[inputchannel] , nrpm_number_lsb[inputchannel],
                    //     number2,
                    //     nrpm_value_msb[inputchannel] , nrpm_value_lsb[inputchannel],
                    //     value2
                    // );

                    // actually set value.

                    uint8_t mapped = false;
                    for(int t=0; t<16; t++) {
                        if (trackToMidiChannel[t] == inputchannel) {
                            int macrocc = (int)number2 - trackBaseCC[t];
                            macrocc -= 8; // input parameter CC's start at 8 too and...
                            if (macrocc >= 0 && macrocc < 24) {
                                // ESP_LOGI("MacroTranslator", "  -> track %d nrpm param %d = %d (baseCC=%d)", t, macrocc, value2, trackBaseCC[t]);
                                this->SetTrackParameter(t, macrocc, value2);
                                mapped = true;
                            }
                        }
                    }

                    if (!mapped) {
                        // ESP_LOGI("MacroTranslator", "  -> no track mapped to MIDI channel %d", inputchannel);
                        this->soundProcessor->handleMidiControlChangeNRPM(inputchannel, number2, value2);
                    }

                } else {
                    // regular control change, continue with normal processing



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
            float volMult = definitions[t].volumeMultiplier;
            soundProcessor->setTrackMachine(t, trackMachineId[t], volMult);
        }
    }

    parseIncomingMidiMessages(pd->midi_bytes, pd->midi_bytes_length);

    for(int t=0; t<16; t++) {
        if (trackDirty[t]) {
            trackDirty[t] = false;

            // TODO: copy mapping instead.
            MacroDeviceDefinition *def = &definitions[t];

            if (def != nullptr) {
                for(int omi = 0; omi < MaxOutputMappings; omi++) {
                    struct MacroDeviceOutputMapping *om = &def->outputMappings[omi];
                    if (om->ctrl < 0) {
                        continue;
                    }

                    int32_t finalvalue = om->startValue;

                    for(int oms=0; oms<MaxOutputMappingSources; oms++) {
                        struct MacroDeviceOutputMappingSource *src = &om->sources[oms];

                        int val = trackParameterValues[t][src->parameterIndex];
                        int curveMax = (om->ctrltype == CtrlType_NRPM) ? 16383 : 127;
                        val = applyCurve(val, curveMax, src->curve);
                        if (src->divider > 0) {
                            finalvalue += (val * src->multiplier) / src->divider;
                        } else {
                            finalvalue += val * src->multiplier;
                        }
                    }

                    int midichannel = trackToMidiChannel[t];
                    int finalcc = om->ctrl + trackBaseCC[t];

                    // ESP_LOGI("MacroTranslator", "Track %d output mapping: CC %d (baseCC=%d) (type %d)= %d",
                    //     t, finalcc, trackBaseCC[t], om->ctrltype, finalvalue);

                    if (om->ctrl != -1) {
                        int midichannel = trackToMidiChannel[t];
                        int finalcc = om->ctrl + trackBaseCC[t];

                        // AUDIO-THREAD: TranslateInput runs on the audio task.
                        // Even gated, this printf corrupts the output buffer.
                        // Diagnostic kept commented for future bring-up.
                        // static uint32_t _macroDiagCtr = 0;
                        // if ((om->ctrl == 8 || om->ctrl == 9) && ((_macroDiagCtr++ % 5000) == 0)) {
                        //     printf("DIAG MacroTranslator: t=%d ch=%d cc=%d+%d=%d val=%ld (src paramVal=%ld)\n",
                        //         t, midichannel, (int)om->ctrl, (int)trackBaseCC[t], (int)finalcc,
                        //         (long)finalvalue, (long)trackParameterValues[t][om->sources[0].parameterIndex]);
                        // }

                        if (om->ctrltype == CtrlType_CC) {
                            if (finalvalue < 0) finalvalue = 0;
                            if (finalvalue > 127) finalvalue = 127;

                            if (finalvalue != outputValues[t][omi]) {
                                outputValues[t][omi] = finalvalue;
                                soundProcessor->handleMidiControlChange(midichannel, finalcc, finalvalue);
                            }
                        } else if (om->ctrltype == CtrlType_NRPM) {
                            if (finalvalue < 0) finalvalue = 0;
                            if (finalvalue > 16383) finalvalue = 16383;

                            if (finalvalue != outputValues[t][omi]) {
                                outputValues[t][omi] = finalvalue;
                                soundProcessor->handleMidiControlChangeNRPM(midichannel, finalcc, finalvalue);
                            }
                        }
                    }
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
        if (definitions[ti].id[0] != '\0') {
            trackjson.AddMember("macro", Value(definitions[ti].id, doc.GetAllocator()), doc.GetAllocator());
        } else {
            trackjson.AddMember("macro", "", doc.GetAllocator());
        }
        // trackjson.AddMember("preset", "", doc.GetAllocator());
        doc["tracks"].PushBack(trackjson, doc.GetAllocator());
    }
    return false;
}

MacroTranslator &MacroTranslator::instance() {
    static MacroTranslator instance;
    return instance;
}