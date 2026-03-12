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

#include "SynthDefinitionDataModel.hpp"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


SynthDefinitionDataModel::SynthDefinitionDataModel() {
    synths.clear();
    tracks.clear();
}

SynthDefinitionDataModel::~SynthDefinitionDataModel() {
}

// #define MB_BUF_SZ 1024

void SynthDefinitionDataModel::ReloadSynthDefinitions() {
    // return;

    #ifndef TBD_SIM
        const std::string MODELJSONFN = "/sdcard/data/synthdefinitions.json";
    #else
        const std::string MODELJSONFN = "../../sdcard_image/data/synthdefinitions.json";
    #endif

    ESP_LOGI("SynthDefinitionDataModel", "Trying to read synth defintition file: %s", MODELJSONFN.c_str());

    Document d;
    loadJSON(d, MODELJSONFN);

    if (d.HasParseError()) {
        ESP_LOGE("SynthDefinitionDataModel", "JSON parse error: %d", d.GetParseError());
        return;
    }

    DeserializeJSON(d);
}

int SynthDefinitionDataModel::GetNumberOfSynthDefinitions() {
    return synths.size();
}

void SynthDefinitionDataModel::GetSynthDeviceDefinitionId(int index, std::string *idOutput) {
}

SynthDefinition *SynthDefinitionDataModel::GetSynthDefinition(const std::string id) {
    for(SynthDefinition *s : synths) {
        if (s->id == id) {
            return s;
        }
    }
    return nullptr;
}

TrackDefinition *SynthDefinitionDataModel::GetTrackDefinition(int index) {
    for(TrackDefinition *t : tracks) {
        if (t->index == index) {
            return t;
        }
    }
    return nullptr;
}

bool SynthDefinitionDataModel::DeserializeJSON(const rapidjson::Value &jsonelement) {
    ESP_LOGI("SynthDefinitionDataModel", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    for (TrackDefinition *t : tracks) {
        delete t;
    }
    tracks.clear();
    for (SynthDefinition *s : synths) {
        delete s;
    }
    synths.clear();

    if (jsonelement.HasMember("machines")) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            SynthDefinition *s = new SynthDefinition();
            if (s->DeserializeJSON(v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized synth definition: #%s \"%s\"", s->id.c_str(), s->name.c_str());
                synths.push_back(s);
            } else {
                delete s;
            }
        }
    }

    if (jsonelement.HasMember("tracks")) {
        for (auto &v : jsonelement["tracks"].GetArray()) {
            TrackDefinition *t = new TrackDefinition();
            if (t->DeserializeJSON(v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized track definition: #%d \"%s\"", t->index, t->name.c_str());
                tracks.push_back(t);
            } else {
                delete t;
            }
        }
    }

    return true;
}

void SynthDefinitionDataModel::SerializeTrackJSON(int index, std::string *output){
    Document d;

    d.SetObject();

    Value tracksarray(kArrayType);
    d.AddMember("tracks", tracksarray, d.GetAllocator());
    for(TrackDefinition *t : tracks) {
        if (t->index == index) {
            d.AddMember("index", t->index, d.GetAllocator());
            d.AddMember("name", Value(t->name.c_str(), d.GetAllocator()), d.GetAllocator());

            Value machinearray(kArrayType);
            for(auto k : t->macroMachineIds) {
                machinearray.PushBack(Value(k.c_str(), d.GetAllocator()), d.GetAllocator());
            }
            d.AddMember("machines", machinearray, d.GetAllocator());
            // d["tracks"].PushBack(d, d.GetAllocator());
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGD("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void SynthDefinitionDataModel::SerializeSynthJSON(const std::string id, std::string *output){
    // Implement serialization logic here
    Document d;
    d.SetObject();

    // Value machinesarray(kArrayType);
    // d.AddMember("machines", machinesarray, d.GetAllocator());
    for(SynthDefinition *s : synths) {
        if (s->id == id ) {

            // Value machinejson(kObjectType);
            d.AddMember("id", Value(s->id.c_str(), d.GetAllocator()), d.GetAllocator());
            d.AddMember("name", Value(s->name.c_str(), d.GetAllocator()), d.GetAllocator());
            // machinejson.AddMember("type", s->type, d.GetAllocator());

            Value paramarray(kArrayType);
            d.AddMember("parameters", paramarray, d.GetAllocator());

            for(auto p : s->parameters) {
                Value param(kObjectType);

                param.AddMember("id", Value(p->id.c_str(), d.GetAllocator()), d.GetAllocator());
                param.AddMember("name", Value(p->name.c_str(), d.GetAllocator()), d.GetAllocator());
                if (p->type == SynthParameterType_CC) {
                    param.AddMember("type", "CC", d.GetAllocator());
                } else if (p->type == SynthParameterType_NRPM) {
                    param.AddMember("type", "NRPM", d.GetAllocator());
                } else {
                    param.AddMember("type", "None", d.GetAllocator());
                }
                param.AddMember("cc", p->cc, d.GetAllocator());
                param.AddMember("default", p->defaultValue, d.GetAllocator());

                d["parameters"].PushBack(param, d.GetAllocator());
            }

            // d["machines"].PushBack(machinejson, d.GetAllocator());
            // if (s->SerializeJSONInto(machinejson, d.GetAllocator())) {
            // }
        }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGD("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void SynthDefinitionDataModel::SerializeListJSON(std::string *output) {
    // Implement serialization logic here
    Document d;

    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(SynthDefinition *s : synths) {
        Value machineid = Value(s->id.c_str(), d.GetAllocator());
        d["machines"].PushBack(machineid, d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGD("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

// bool SynthDefinitionDataModel::UpdateDefinitionJSON(const std::string &jsonstring) {
//     Document d;
//     if (d.Parse(jsonstring.c_str()).HasParseError()) {
//         ESP_LOGE("SynthDefinitionDataModel", "Failed to parse JSON string: %s", jsonstring.c_str());
//         return false;
//     }

//     #ifndef TBD_SIM
//         const std::string MODELJSONFN = "/sdcard/data/synthdefinitions.json";
//     #else
//         const std::string MODELJSONFN = "../../sdcard_image/data/synthdefinitions.json";
//     #endif

//     fp = fopen(MODELJSONFN.c_str(), "w");
//     if (fp == NULL) {
//         ESP_LOGE("MacroSoundPresetDataModel", "could not open file %s", MODELJSONFN.c_str());
//         return false;
//     }
//     fwrite(jsonstring.c_str(), 1, jsonstring.size(), fp);
//     fclose(fp);

//     return true;
// }

