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
#include "MacroSoundPreset.hpp"
#include "ctagResources.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void MacroSoundPresetDataModel::Init() {
    presets = (MacroSoundPreset*) heap_caps_malloc(sizeof(MacroSoundPreset) * MaxSoundPresets, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);
    groups = (MacroSoundPresetGroup*) heap_caps_malloc(sizeof(MacroSoundPresetGroup) * MaxSoundPresetGroups, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);
    for(int i=0; i<MaxSoundPresets; i++) {
        MacroSoundPresetUtils::MacroSoundPreset_Reset(&presets[i]);
    }
    for(int i=0; i<MaxSoundPresetGroups; i++) {
        MacroSoundPresetUtils::MacroSoundPresetGroup_Reset(&groups[i]);
    }
}

int compareGroups(const void *a, const void *b) {
    struct MacroSoundPresetGroup *ga = (struct MacroSoundPresetGroup*)a;
    struct MacroSoundPresetGroup *gb = (struct MacroSoundPresetGroup*)b;
    return strcmp(ga->displayName, gb->displayName);
}

void MacroSoundPresetDataModel::ReloadSoundPresets() {
    ESP_LOGI("MacroSoundPresetDataModel", "Trying to read macro sound preset file");

    ESP_LOGI("MacroSoundPresetDataModel", "Before reload: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    Document d;

    for(int i=0; i<MaxSoundPresets; i++) {
        MacroSoundPresetUtils::MacroSoundPreset_Reset(&presets[i]);
    }
    for(int i=0; i<MaxSoundPresetGroups; i++) {
        MacroSoundPresetUtils::MacroSoundPresetGroup_Reset(&groups[i]);
    }

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    int pindex = 0;
    int gindex = 0;
    const std::string path = CTAG::RESOURCES::sdcardRoot + "/data/macrosoundpresets";
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);

            Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                if(MacroSoundPresetUtils::MacroSoundPreset_DeserializeJSON(&presets[pindex], d)) {
                    ESP_LOGI("MacroSoundPresetDataModel", "Got sound preset: %d id: %s \"%s\"", pindex, presets[pindex].id, presets[pindex].displayName);

                    // Find existing or create new group
                    gindex = -1;
                    for(int i=0; i<MaxSoundPresetGroups; i++) {
                        if (strcmp(groups[i].id, presets[pindex].groupName) == 0) {
                            gindex = i;
                            break;
                        }
                    }
                    if (gindex == -1) {
                        for(int i=0; i<MaxSoundPresetGroups; i++) {
                            if (groups[i].id[0] == '\0') {
                                gindex = i;
                                break;
                            }
                        }
                        strcpy(groups[gindex].id, presets[pindex].groupName);
                        strcpy(groups[gindex].displayName, presets[pindex].groupName);
                        groups[gindex].validTracksBitmask = 0;
                        groups[gindex].numFileIds = 0;
                    }

                    strcpy(groups[gindex].fileIds[groups[gindex].numFileIds], presets[pindex].id);
                    groups[gindex].numFileIds++;

                    MacroDeviceDefinition *macrodef = MacroDeviceDefinitionDataModel::instance()
                        .GetMacroDeviceDefinition((const char *)&presets[pindex].macroDeviceId);
                    if (macrodef == nullptr) {
                        ESP_LOGE("MacroSoundPresetDataModel", "  Could not find macro device definition with id %s for preset %s", presets[pindex].macroDeviceId, presets[pindex].id);
                    } else {
                        ESP_LOGI("MacroSoundPresetDataModel", "  Found macro device definition with id %s for preset %s", presets[pindex].macroDeviceId, presets[pindex].id);
                        // figure out which tracks this preset is valid on.
                        for (int i = 0; i < 16; i++) {
                            TrackDefinition *trackdef = SynthDefinitionDataModel::instance()->GetTrackDefinition(i);
                            if (trackdef != nullptr) {
                                for(int j=0; j<MaxTrackDefinitionMachineIds; j++) {
                                    if (trackdef->macroMachineIds[j][0] != '\0') {
                                        if (strcmp(trackdef->macroMachineIds[j], macrodef->synthId) == 0) {
                                            ESP_LOGD("MacroSoundPresetDataModel", "    Marking as supported on track %d (pindex %d, gindex %d)\n", i, pindex, gindex);
                                            presets[pindex].validTracksBitmask |= (1 << i);
                                            groups[gindex].validTracksBitmask |= (1 << i);
                                        }
                                    }
                                }
                            }
                        }
                        ESP_LOGD("MacroSoundPresetDataModel", "    gindex %d trackmask %d", gindex, groups[gindex].validTracksBitmask);
                        ESP_LOGD("MacroSoundPresetDataModel", "    pindex %d trackmask %d", pindex, presets[pindex].validTracksBitmask);
                    }

                    pindex ++;
                } else {
                    ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", fn.c_str());
                }
            } else {
                ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse preset file: %s", fn.c_str());
            }
        }
        closedir(dir);
        presetsUsed = pindex;

        // TODO: Sort groups

        int gcount = 0;
        // find number of groups
        for(int g=0; g<MaxSoundPresetGroups; g++) {
            if (groups[g].id[0] != '\0') {
                gcount++;
            }
        }
        qsort(groups, gcount, sizeof(struct MacroSoundPresetGroup), compareGroups);
        groupsUsed = gcount;
    } else {
        ESP_LOGE("MacroSoundPresetDataModel", "Could not open directory %s", path.c_str());
    }

    ESP_LOGI("MacroSoundPresetDataModel", "After reload: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

void MacroSoundPresetDataModel::LoadMacroSoundPreset(MacroSoundPreset *target, std::string id) {
    const std::string path = CTAG::RESOURCES::sdcardRoot + "/data/macrosoundpresets";
    const std::string filename = path + "/" + id + ".json";

    Document d;
    loadJSON(d, filename);
    if(d.HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON file %s", filename.c_str());
        return;
    }

    if(!MacroSoundPresetUtils::MacroSoundPreset_DeserializeJSON(target, d)) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize macro sound preset from file %s", filename.c_str());
        return;
    }
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

    doc.AddMember("track", trackIndex, doc.GetAllocator());

    SerializeListInto(trackIndex, doc);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    ESP_LOGI("MacroSoundPresetDataModel", "GetPresetIndexJson JSON: %s", buffer.GetString());

    output->assign(buffer.GetString());
}

void MacroSoundPresetDataModel::SerializeListJSON(std::string *output) {
    Document doc;

    doc.SetObject();

    Value presetsarray(kArrayType);
    doc.AddMember("presets", presetsarray, doc.GetAllocator());
    for(int i=0; i<MaxSoundPresets; i++) {
        if (presets[i].id[0] == '\0') {
            continue;
        }
        doc["presets"].PushBack(Value(presets[i].id, doc.GetAllocator()), doc.GetAllocator());
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

    const std::string path = CTAG::RESOURCES::sdcardRoot + "/data/macrosoundpresets";
    const std::string filename = path + "/" + id + ".json";

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
    const std::string path = CTAG::RESOURCES::sdcardRoot + "/data/macrosoundpresets";
    const std::string filename = path + "/" + id + ".json";

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
    const std::string path = CTAG::RESOURCES::sdcardRoot + "/data/macrosoundpresets";
    const std::string filename = path + "/" + id + ".json";
    ESP_LOGI("MacroSoundPresetDataModel", "Deleting file: %s", filename.c_str());
    unlink(filename.c_str());
}

void MacroSoundPresetDataModel::SerializeListInto(int trackIndex, rapidjson::Document &doc) {
    Value groupsarray(kArrayType);
    doc.AddMember("presetgroups", groupsarray, doc.GetAllocator());

    for(int gi=0; gi<MaxSoundPresetGroups; gi++) {
        if (groups[gi].id[0] == '\0') {
            continue;
        }

        if (!(groups[gi].validTracksBitmask & (1 << trackIndex))) {
            continue;
        }

        Value groupobj(kObjectType);
        groupobj.AddMember("id", Value(groups[gi].id, doc.GetAllocator()), doc.GetAllocator());
        groupobj.AddMember("name", Value(groups[gi].displayName, doc.GetAllocator()), doc.GetAllocator());

        Value presetsarray(kArrayType);
        groupobj.AddMember("presets", presetsarray, doc.GetAllocator());

        for(int pi=0; pi<MaxSoundPresets; pi++) {
            if (presets[pi].id[0] == '\0') {
                continue;
            }

            if (!(presets[pi].validTracksBitmask & (1 << trackIndex))) {
                continue;
            }

            if (strcmp(presets[pi].groupName, groups[gi].displayName) != 0) {
                continue;
            }

            Value presetobj(kObjectType);
            presetobj.AddMember("id", Value(presets[pi].id, doc.GetAllocator()), doc.GetAllocator());
            presetobj.AddMember("name", Value(presets[pi].displayName, doc.GetAllocator()), doc.GetAllocator());
            // TODO: Use name
            groupobj["presets"].PushBack(presetobj, doc.GetAllocator());
        }

        doc["presetgroups"].PushBack(groupobj, doc.GetAllocator());
    }
}

bool MacroSoundPresetDataModel::PutSamplePresetJSON(const string &presetJSON) {
    Document d;
    d.Parse(presetJSON.c_str());
    if (d.HasParseError()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to parse JSON string: %s", presetJSON.c_str());
        return false;
    }

    MacroSoundPreset mp;
    if (!MacroSoundPresetUtils::MacroSoundPreset_DeserializeJSON(&mp, d)) {
        ESP_LOGE("MacroSoundPresetDataModel", "Failed to deserialize MacroSoundPreset from JSON string: %s", presetJSON.c_str());
        return false;
    }

    const std::string path = CTAG::RESOURCES::sdcardRoot +
        "/data/macrosoundpresets/" + std::string(mp.id) + ".json";

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

MacroSoundPresetDataModel &MacroSoundPresetDataModel::instance() {
    static MacroSoundPresetDataModel instance;
    return instance;
}