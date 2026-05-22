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
#include "EngineDefinitionDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"
#include "MacroSoundPreset.hpp"
#include "ctagResources.hpp"
#include "StorageOverlay.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void MacroSoundPresetDataModel::Init() {
    arrayMutex = xSemaphoreCreateMutex();
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

    xSemaphoreTake(arrayMutex, portMAX_DELAY);

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

    int pindex = 0;
    int gindex = 0;
    // Use overlay: merged listing of /user/presets/ + /factory/presets/
    auto presetFiles = CTAG::STORAGE::listMergedDir(CTAG::STORAGE::DIR_PRESETS);
    for (const auto &fn : presetFiles) {
        {
            std::string resolvedPath = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_PRESETS, fn);
            if (resolvedPath.empty()) continue;

            Document d;
            loadJSON(d, resolvedPath);
            if(!d.HasParseError()) {
                if(MacroSoundPresetUtils::MacroSoundPreset_DeserializeJSON(&presets[pindex], d)) {
                    ESP_LOGD("MacroSoundPresetDataModel", "Got sound preset: %d id: %s \"%s\"", pindex, presets[pindex].id, presets[pindex].displayName);

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
                        ESP_LOGD("MacroSoundPresetDataModel", "  Found macro device definition with id %s for preset %s", presets[pindex].macroDeviceId, presets[pindex].id);
                        // figure out which tracks this preset is valid on.
                        // Iterate up to track 18 (FX1=16, FX2=17, Master=18) so
                        // fxdelay / fxreverb / fxmaster presets get their bits
                        // set and the SoundPresetScreen picker can list them
                        // when invoked from the FX1 / FX2 / Master pages.
                        for (int i = 0; i < 19; i++) {
                            struct TrackDefinition *trackdef = EngineDefinitionDataModel::instance()->GetTrackDefinition(i);
                            if (trackdef != nullptr) {
                                for(int j=0; j<MaxTrackDefinitionEngineIds; j++) {
                                    if (trackdef->engineIdStr[j][0] != '\0') {
                                        if (strcmp(trackdef->engineIdStr[j], macrodef->synthId) == 0) {
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
    }
    presetsUsed = pindex;

    // Sort groups
    int gcount = 0;
    for(int g=0; g<MaxSoundPresetGroups; g++) {
        if (groups[g].id[0] != '\0') {
            gcount++;
        }
    }
    qsort(groups, gcount, sizeof(struct MacroSoundPresetGroup), compareGroups);
    groupsUsed = gcount;

    ESP_LOGI("MacroSoundPresetDataModel", "Loaded %d sound presets in %d groups", presetsUsed, groupsUsed);
    ESP_LOGD("MacroSoundPresetDataModel", "After reload: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
        heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    xSemaphoreGive(arrayMutex);
}

void MacroSoundPresetDataModel::LoadMacroSoundPreset(MacroSoundPreset *target, std::string id) {
    // Overlay: check /user/presets/ then /factory/presets/
    const std::string filename = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_PRESETS, id + ".json");
    if (filename.empty()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Preset not found in overlay: %s", id.c_str());
        return;
    }

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
    xSemaphoreTake(arrayMutex, portMAX_DELAY);
    Document doc;

    doc.SetObject();

    doc.AddMember("track", trackIndex, doc.GetAllocator());

    SerializeListInto(trackIndex, doc);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    ESP_LOGI("MacroSoundPresetDataModel", "GetPresetIndexJson JSON: %s", buffer.GetString());

    output->assign(buffer.GetString());
    xSemaphoreGive(arrayMutex);
}

void MacroSoundPresetDataModel::SerializeListJSON(std::string *output) {
    xSemaphoreTake(arrayMutex, portMAX_DELAY);
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
    xSemaphoreGive(arrayMutex);
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

    // Write to /user/presets/ (user data layer)

    const std::string filename = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_PRESETS, id + ".json");

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
    // Overlay: check /user/presets/ then /factory/presets/
    const std::string filename = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_PRESETS, id + ".json");
    if (filename.empty()) {
        ESP_LOGE("MacroSoundPresetDataModel", "Preset not found in overlay: %s", id.c_str());
        output->assign("");
        return;
    }

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
    // Only delete from /user/presets/ (factory presets are immutable)
    const std::string filename = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_PRESETS, id + ".json");
    ESP_LOGI("MacroSoundPresetDataModel", "Deleting file: %s", filename.c_str());
    unlink(filename.c_str());
}

void MacroSoundPresetDataModel::SerializeListInto(int trackIndex, rapidjson::Document &doc) {
    Value groupsarray(kArrayType);
    doc.AddMember("presetgroups", groupsarray, doc.GetAllocator());

    // Group presets by machine (synth engine) like the web UI TrackDefaults dialog.
    // Hierarchy: Track → machines (from TrackDefinition) → presets using macros for that machine.
    struct TrackDefinition *trackDef = EngineDefinitionDataModel::instance()->GetTrackDefinition(trackIndex);
    if (trackDef == nullptr) return;

    for (int mi = 0; mi < MaxTrackDefinitionEngineIds; mi++) {
        if (trackDef->engineIdStr[mi][0] == '\0') continue;
        const char *engineId = trackDef->engineIdStr[mi];

        // Skip empty placeholder machines (same filter as web UI)
        if (strcmp(engineId, "nodrum") == 0 ||
            strcmp(engineId, "nosynth") == 0 ||
            strcmp(engineId, "nofx") == 0) continue;

        // Get machine display name from SynthDefinition
        SharedEngineDefinition *synthDef = EngineDefinitionDataModel::instance()->GetSynthDefinition(std::string(engineId));
        const char *machineName = synthDef ? synthDef->name : engineId;

        Value groupobj(kObjectType);
        groupobj.AddMember("id", Value(engineId, doc.GetAllocator()), doc.GetAllocator());
        groupobj.AddMember("name", Value(machineName, doc.GetAllocator()), doc.GetAllocator());

        Value presetsarray(kArrayType);
        groupobj.AddMember("presets", presetsarray, doc.GetAllocator());

        // Find all presets whose macro definition targets this synth engine
        for (int pi = 0; pi < presetsUsed; pi++) {
            if (presets[pi].id[0] == '\0') continue;
            if (!(presets[pi].validTracksBitmask & (1 << trackIndex))) continue;

            // Look up the preset's macro definition to check its synthId
            MacroDeviceDefinition *macroDef = MacroDeviceDefinitionDataModel::instance()
                .GetMacroDeviceDefinition(presets[pi].macroDeviceId);
            if (macroDef == nullptr) continue;
            if (strcmp(macroDef->synthId, engineId) != 0) continue;

            Value presetobj(kObjectType);
            presetobj.AddMember("id", Value(presets[pi].id, doc.GetAllocator()), doc.GetAllocator());
            presetobj.AddMember("name", Value(presets[pi].displayName, doc.GetAllocator()), doc.GetAllocator());
            groupobj["presets"].PushBack(presetobj, doc.GetAllocator());
        }

        // Only include machine if it has at least one preset
        if (groupobj["presets"].Size() > 0) {
            doc["presetgroups"].PushBack(groupobj, doc.GetAllocator());
        }
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

    // Write to /user/presets/ (user data layer)
    const std::string path = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_PRESETS, std::string(mp.id) + ".json");

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