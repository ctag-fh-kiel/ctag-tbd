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
    ESP_LOGD("SynthDefinitionDataModel", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    int index = 0;
    if (jsonelement.HasMember("machines")) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            if (CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_DeserializeJSON(&synths[index], v)) {
                ESP_LOGD("SynthDefinitionDataModel", "Deserialized synth definition: #%d id %s \"%s\"", index, synths[index].id, synths[index].name);
                index ++;
            }
        }
    }
    int synthCount = index;

    index = 0;
    if (jsonelement.HasMember("tracks")) {
        for (auto &v : jsonelement["tracks"].GetArray()) {
            if (CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_DeserializeJSON(&tracks[index], v)) {
                ESP_LOGD("SynthDefinitionDataModel", "Deserialized track definition #%d index %d \"%s\"", index, tracks[index].index, tracks[index].name);
                // tracks.push_back(t);
                index ++;
            }
        }
    }

    ESP_LOGI("SynthDefinitionDataModel", "Loaded %d synth definitions, %d track definitions", synthCount, index);
    return true;
}

#pragma GCC diagnostic ignored "-Wstringop-truncation"
void SynthDefinitionDataModel::WriteListResponse(struct GetEngineDefinitionIdListResponse *r) {
    r->numEngines = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id[0] != '\0') {
            strncpy((char *)&r->engineIds[r->numEngines], (const char *)&synths[i].id, 8);
            r->numEngines ++;
            if (r->numEngines >= MAX_SYNTHS)
                break;
        }
    }
}

void SynthDefinitionDataModel::WritePageResponse(const struct GetEngineDefinitionsPageRequest *request, struct GetEngineDefinitionsPageResponse *response) {
    response->offset = request->offset;
    response->totalEngineDefinitions = GetNumberOfSynthDefinitions();
    response->returnedEngineDefinitions = 0;

    for(int k=0; k<MaxEngineDefinitionsPerPage; k++) {
        int i = request->offset + k;
        if (i >= MAX_SYNTHS) {
            break;
        }
        if (synths[i].id[0] == '\0') {
            continue;
        }

        response->engineDefinitions[k].id = 0;
        strcpy( response->engineDefinitions[k].idStr, synths[i].id);
        strncpy(response->engineDefinitions[k].name, synths[i].name, 8);

        for(int p=0; p<MaxSynthDefinitionParameters; p++) {
            // response->engineDefinitions[k].parameters[p].index = synths[i].parameters[p].index;
            response->engineDefinitions[k].parameters[p].type = EngineParameterType_None;
            if (synths[i].parameters[p].type == SynthParameterType_CC) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_CC;
            } else if (synths[i].parameters[p].type == SynthParameterType_NRPM) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_NRPM;
            }

            response->engineDefinitions[k].parameters[p].defaultValue = synths[i].parameters[p].defaultValue;
            response->engineDefinitions[k].parameters[p].relCC = synths[i].parameters[p].cc;

            strncpy(response->engineDefinitions[k].parameters[p].name, synths[i].parameters[p].name, 32);
        }

        response->returnedEngineDefinitions ++;
    }
}

// void SynthDefinitionDataModel::SerializeListJSON(std::string *output) {
//     Document d;

//     d.SetObject();

//     Value machinesarray(kArrayType);
//     d.AddMember("machines", machinesarray, d.GetAllocator());
//     for(int i=0; i<MAX_SYNTHS; i++) {
//         if (synths[i].id[0] == '\0') {
//             continue;
//         }

//         Value machineid = Value(synths[i].id, d.GetAllocator());
//         d["machines"].PushBack(machineid, d.GetAllocator());
//     }

//     StringBuffer buffer;
//     Writer<StringBuffer> writer(buffer);
//     d.Accept(writer);
//     ESP_LOGD("SynthDefinitionDataModel", "JSON string %s", buffer.GetString());

//     output->assign(buffer.GetString());
// }

static SynthDefinitionDataModel g_synthdef_instance;

SynthDefinitionDataModel *SynthDefinitionDataModel::instance() {
    return &g_synthdef_instance;
}