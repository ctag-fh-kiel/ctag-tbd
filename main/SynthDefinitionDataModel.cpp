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
#include "ctagResources.hpp"
#include "StorageOverlay.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void SynthDefinitionDataModel::Init() {
    synths = (struct SynthDefinition *) heap_caps_malloc(MAX_SYNTHS * sizeof(struct SynthDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    tracks = (struct TrackDefinition *) heap_caps_malloc(MAX_TRACKS * sizeof(struct TrackDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

    for(int i=0; i<MAX_SYNTHS; i++) {
        CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_Reset(&synths[i]);
    }

    for(int i=0; i<MAX_TRACKS; i++) {
        CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_Reset(&tracks[i]);
    }
}

// #define MB_BUF_SZ 1024

void SynthDefinitionDataModel::ReloadSynthDefinitions() {
    // return;

    // synthdefinitions.json is a factory resource (read-only, describes available synths)
    const std::string MODELJSONFN = CTAG::STORAGE::factoryPath() + "/synthdefinitions.json";

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
    int count = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id[0] != '\0') {
            count ++;
        }
    }
    return count;
}

void SynthDefinitionDataModel::GetSynthDeviceDefinitionId(int index, std::string *idOutput) {
}

SynthDefinition *SynthDefinitionDataModel::GetSynthDefinition(const std::string id) {
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id == id) {
            return &synths[i];
        }
    }
    return nullptr;
}

TrackDefinition *SynthDefinitionDataModel::GetTrackDefinition(int index) {
    for(int i=0; i<MAX_TRACKS; i++) {
        if (tracks[i].index == index) {
            return &tracks[i];
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

    int index = 0;
    if (jsonelement.HasMember("machines")) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            if (CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_DeserializeJSON(&synths[index], v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized synth definition: #%d id %s \"%s\"", index, synths[index].id, synths[index].name);
                index ++;
            }
        }
    }

    index = 0;
    if (jsonelement.HasMember("tracks")) {
        for (auto &v : jsonelement["tracks"].GetArray()) {
            if (CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_DeserializeJSON(&tracks[index], v)) {
                ESP_LOGI("SynthDefinitionDataModel", "Deserialized track definition #%d index %d \"%s\"", index, tracks[index].index, tracks[index].name);
                // tracks.push_back(t);
                index ++;
            }
        }
    }

    return true;
}

void SynthDefinitionDataModel::SerializeListJSON(std::string *output) {
    // Implement serialization logic here
    Document d;

    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(int i=0; i<MAX_SYNTHS; i++) {
        Value machineid = Value(synths[i].id, d.GetAllocator());
        d["machines"].PushBack(machineid, d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGD("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

static SynthDefinitionDataModel g_synthdef_instance;

SynthDefinitionDataModel *SynthDefinitionDataModel::instance() {
    return &g_synthdef_instance;
}