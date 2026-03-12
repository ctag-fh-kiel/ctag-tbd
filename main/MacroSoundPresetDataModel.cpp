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

#include "MacroSoundPresetDataModel.hpp"
#include "MacroSoundPresetGroup.hpp"
#include "MacroSoundPreset.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"
#include "TrackDefinition.hpp"
#include "SynthDefinition.hpp"
#include "SynthDefinitionDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"
#include "ctagResources.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroSoundPresetDataModel::MacroSoundPresetDataModel() {
    presets.clear();
    groups.clear();
}

MacroSoundPresetDataModel::~MacroSoundPresetDataModel() {
}

int compareGroups(const void *a, const void *b) {
    MacroSoundPresetGroup *ga = *(MacroSoundPresetGroup**)a;
    MacroSoundPresetGroup *gb = *(MacroSoundPresetGroup**)b;
    return ga->displayName.compare(gb->displayName);
}

void MacroSoundPresetDataModel::ReloadSoundPresets(
    MacroDeviceDefinitionDataModel *macromodel,
    SynthDefinitionDataModel *synthmodel
) {
    ESP_LOGI("MacroSoundPresetDataModel", "Trying to read macro sound preset file");

    ESP_LOGI("MacroSoundPresetDataModel", "Before clear: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    Document d;

    for(MacroSoundPreset *p : presets) {
        delete p;
    }
    presets.clear();
    for(MacroSoundPresetGroup *g : groups) {
        delete g;
    }
    groups.clear();

    ESP_LOGI("MacroSoundPresetDataModel", "After clear: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);

            ESP_LOGI("MacroSoundPresetDataModel", "In loop: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
                heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

            Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                MacroSoundPreset *preset = new MacroSoundPreset();
                if(preset->DeserializeJSON(d)) {
                    ESP_LOGI("MacroSoundPresetDataModel", "Got sound preset: #%s \"%s\"", preset->id.c_str(), preset->displayName.c_str());
                    presets.push_back(preset);

                    // Find existing or create new group
                    MacroSoundPresetGroup *group = nullptr;
                    for(MacroSoundPresetGroup *g : groups) {
                        if (g->id == preset->groupName) {
                            group = g;
                            break;
                        }
                    }
                    if (group != nullptr) {
                        group->fileIds.push_back(preset->id);
                    } else {
                        ESP_LOGI("MacroSoundPresetDataModel", "Found new group: \"%s\"", preset->groupName.c_str());
                        group = new MacroSoundPresetGroup();
                        group->id = preset->groupName;
                        group->displayName = preset->groupName;
                        group->fileIds.push_back(preset->id);
                        groups.push_back(group);
                    }

                    MacroDeviceDefinition *macrodef = macromodel->LoadMacroDeviceDefinition(preset->macroDeviceId);
                    if (macrodef == nullptr) {
                        ESP_LOGE("MacroSoundPresetDataModel", "  Could not find macro device definition with id %s for preset %s", preset->macroDeviceId.c_str(), preset->id.c_str());
                    } else {
                        ESP_LOGI("MacroSoundPresetDataModel", "  Found macro device definition with id %s for preset %s", preset->macroDeviceId.c_str(), preset->id.c_str());
                        // figure out which tracks this preset is valid on.
                        for (int i = 0; i < 16; i++) {
                            TrackDefinition *trackdef = synthmodel->GetTrackDefinition(i);
                            if (trackdef != nullptr) {
                                for(std::string mid : trackdef->macroMachineIds) {
                                    if (mid == macrodef->synthId) {
                                        // ESP_LOGI("MacroSoundPresetDataModel", "    Track %d has macro machine id %s", i, mid.c_str());
                                        preset->validTracks.insert(i);
                                        group->validTracks.insert(i);
                                    }
                                }
                            }
                        }
                        delete macrodef;
                    }
                } else {
                    ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", fn.c_str());
                    delete preset;
                }
            } else {
                ESP_LOGI("MacroSoundPresetDataModel", "Failed to parse preset file: %s", fn.c_str());
            }
        }
        closedir(dir);

        // TODO: Sort groups
        qsort(groups.data(), groups.size(), sizeof(MacroSoundPresetGroup*), compareGroups);
    } else {
        ESP_LOGE("MacroSoundPresetDataModel", "Could not open directory %s", path.c_str());
    }

    ESP_LOGI("MacroSoundPresetDataModel", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

MacroSoundPreset *MacroSoundPresetDataModel::LoadMacroSoundPreset(std::string id) {
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";

    Document d;
    loadJSON(d, filename);
    if(d.HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON file %s", filename.c_str());
        return nullptr;
    }

    MacroSoundPreset *preset = new MacroSoundPreset();
    if(!preset->DeserializeJSON(d)) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", filename.c_str());
        delete preset;
        return nullptr;
    }

    return preset;
}

int MacroSoundPresetDataModel::GetNumberOfSoundPresetGroups() {
    return 0;
}

void MacroSoundPresetDataModel::GetMacroSoundPresetGroupId(int index, std::string *idOutput) {
    idOutput->assign("xyz");
}

int MacroSoundPresetDataModel::GetNumberOfSoundPresets() {
    return 0;
}

void MacroSoundPresetDataModel::GetPresetIndexJson(int trackIndex, std::string *output) {
    Document doc;

    doc.SetObject();

    SerializeListInto(trackIndex, doc);

    doc.AddMember("track", trackIndex, doc.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    ESP_LOGD("MacroSoundPresetDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void MacroSoundPresetDataModel::SerializeListJSON(std::string *output) {
    Document doc;

    doc.SetObject();



    Value presetsarray(kArrayType);
    doc.AddMember("presets", presetsarray, doc.GetAllocator());
    for(MacroSoundPreset *s : presets) {
        doc["presets"].PushBack(Value(s->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
        // Document d2;
        // if (s->SerializeJSONInto(d2)) {
        //     Value copyOfd2(d2, doc.GetAllocator());
        //     doc["presets"].PushBack(copyOfd2.Move(), doc.GetAllocator());
        // } else {
        //     ESP_LOGE("MacroSoundPresetDataModel", "Failed to serialize macro sound preset #%s \"%s\"", s->id.c_str(), s->displayName.c_str());
        // }
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    ESP_LOGD("MacroSoundPresetDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
}

bool MacroSoundPresetDataModel::UpdatePreset(const std::string &jsonString) {
    Document d;
    if (d.Parse(jsonString.c_str()).HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON string: %s", jsonString.c_str());
        return false;
    }

    if (!d.HasMember("id")) {
        ESP_LOGE("MacroSoundPresetDataModel", "JSON string does not contain 'id' field: %s", jsonString.c_str());
        return false;
    }

    std::string id = d["id"].GetString();

    // just save the file now when we know the id

    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";

    fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could not open file %s", filename.c_str());
        return false;
    }
    fwrite(jsonString.c_str(), 1, jsonString.size(), fp);
    fclose(fp);

    // ReloadSoundPresets();

    return true;
}


void MacroSoundPresetDataModel::SerializeItemJSON(const std::string &id, std::string *output) {
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";

    output->assign("");

    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could not open file %s", filename.c_str());
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = (char *) heap_caps_malloc(filesize + 32, MALLOC_CAP_SPIRAM);
    if (content == NULL) {
        ESP_LOGE("MacroSoundPresetDataModel", "could allocate memory");
        // output->assign("");
        fclose(fp);
        return;
    }

    fread(content, 1, filesize, fp);
    fclose(fp);

    content[filesize] = '\0';

    ESP_LOGD("MacroSoundPresetDataModel", "JSON string %s", content);

    output->assign(content);

    heap_caps_free(content);
}

void MacroSoundPresetDataModel::DeleteItem(const std::string &id) {
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrosoundpresets"));
    std::string filename = path + "/" + id + ".json";
    ESP_LOGI("MacroSoundPresetDataModel", "Deleting file: %s", filename.c_str());
    unlink(filename.c_str());
}

bool MacroSoundPresetDataModel::SerializeListInto(int trackIndex, rapidjson::Document &doc) {
    Value groupsarray(kArrayType);
    doc.AddMember("presetgroups", groupsarray, doc.GetAllocator());

    for(MacroSoundPresetGroup *g : groups) {
        if (!g->validTracks.contains(trackIndex)) {
            continue;
        }

        Value groupobj(kObjectType);
        groupobj.AddMember("id", Value(g->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
        groupobj.AddMember("name", Value(g->displayName.c_str(), doc.GetAllocator()), doc.GetAllocator());

        Value presetsarray(kArrayType);
        groupobj.AddMember("presets", presetsarray, doc.GetAllocator());

        for(MacroSoundPreset *p : presets) {
            if (!p->validTracks.contains(trackIndex)) {
                continue;
            }

            if (p->groupName != g->displayName) {
                continue;
            }

            Value presetobj(kObjectType);
            presetobj.AddMember("id", Value(p->id.c_str(), doc.GetAllocator()), doc.GetAllocator());
            presetobj.AddMember("name", Value(p->displayName.c_str(), doc.GetAllocator()), doc.GetAllocator());
            // TODO: Use name
            groupobj["presets"].PushBack(presetobj, doc.GetAllocator());
        }

        doc["presetgroups"].PushBack(groupobj, doc.GetAllocator());
    }

    return false;
}

bool MacroSoundPresetDataModel::PutSamplePresetJSON(const string &presetJSON) {
    Document d;
    d.Parse(presetJSON.c_str());
    if (d.HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON string: %s", presetJSON.c_str());
        return false;
    }

    MacroSoundPreset mp;
    if (!mp.DeserializeJSON(d)) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize MacroSoundPreset from JSON string: %s", presetJSON.c_str());
        return false;
    }

    std::string id = mp.id;
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot +
        std::string("/data/macrosoundpresets/") + id + ".json");

    FILE *f = fopen(path.c_str(), "w");
    if (!f) {
        ESP_LOGW("SpiAPI", "GetTrackDefaultPresets: trackdefaults.json not found, returning {}");
        return false;
    }

    fwrite(presetJSON.c_str(), 1, presetJSON.size(), f);
    fclose(f);
    ESP_LOGI("SpiAPI", "GetTrackDefaultPresets: wrote %ld bytes to %s", presetJSON.size(), path.c_str());

    return true;
}