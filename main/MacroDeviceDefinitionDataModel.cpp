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

#include "MacroDeviceDefinitionDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ctagResources.hpp"
#include "StorageOverlay.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void MacroDeviceDefinitionDataModel::Init() {
    arrayMutex = xSemaphoreCreateMutex();
    definitions = (struct MacroDeviceDefinition *)heap_caps_malloc(sizeof(struct MacroDeviceDefinition) * MaxMacroDeviceDefinitions, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM);
    for(int i=0; i<MaxMacroDeviceDefinitions; i++) {
        MacroDeviceDefinitionUtils::MacroDeviceDefinition_Reset(&definitions[i]);
    }
}

void MacroDeviceDefinitionDataModel::ReloadMachineDefinitions() {
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Trying to read macro device definition file");

    xSemaphoreTake(arrayMutex, portMAX_DELAY);

    for(int i=0; i<MaxMacroDeviceDefinitions; i++) {
        MacroDeviceDefinitionUtils::MacroDeviceDefinition_Reset(&definitions[i]);
    }

    int index = 0;
    // Use overlay: merged listing of /user/macros/ + /factory/macros/
    auto macroFiles = CTAG::STORAGE::listMergedDir(CTAG::STORAGE::DIR_MACROS);
    for (const auto &fn : macroFiles) {
        {
            std::string resolvedPath = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_MACROS, fn);
            if (resolvedPath.empty()) continue;

            Document d;
            loadJSON(d, resolvedPath);
            if(!d.HasParseError()) {
                if( MacroDeviceDefinitionUtils::MacroDeviceDefinition_DeserializeJSON(&definitions[index], d)) {
                    ESP_LOGI("MacroDeviceDefinitionDataModel", "Deserialized macro device definition: #%s \"%s\"", definitions[index].id, definitions[index].name);
                    index ++;
                } else {
                    ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to deserialize macro device definition from file %s", fn.c_str());
                }
            } else {
                ESP_LOGI("MacroDeviceDefinitionDataModel", "Failed to parse file: %s", fn.c_str());

            }
        }
    }

    xSemaphoreGive(arrayMutex);
}

bool MacroDeviceDefinitionDataModel::ReloadSingleDefinition(const std::string &id) {
    xSemaphoreTake(arrayMutex, portMAX_DELAY);
    // Overlay: check /user/macros/ then /factory/macros/
    const std::string path = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_MACROS, id + ".json");
    if (path.empty()) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Macro def not found in overlay: %s", id.c_str());
        xSemaphoreGive(arrayMutex);
        return false;
    }
    Document d;
    loadJSON(d, path);
    if (d.HasParseError()) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to parse %s", path.c_str());
        xSemaphoreGive(arrayMutex);
        return false;
    }

    int index = 0;
    for(int i=0; i<MaxMacroDeviceDefinitions; i++) {
        if (strcmp(definitions[i].id, id.c_str()) == 0) {
            // Found existing definition with matching ID, update it
            index = i;
            break;
        }
    }

    // MacroDeviceDefinition *newDef = new MacroDeviceDefinition();
    if (!MacroDeviceDefinitionUtils::MacroDeviceDefinition_DeserializeJSON(&definitions[index], d)) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to deserialize %s", id.c_str());
        xSemaphoreGive(arrayMutex);
        return false;
    }

    // // Replace existing entry or append
    // for (size_t i = 0; i < MaxMacroDeviceDefinitions; i++) {
    //     if (definitions[i].id[0] == '\0') {
    //         // Empty slot, add new definition here
    //         continue;
    //     }

    //     if (strcmp(definitions[i].id, id.c_str()) == 0) {
    //         definitions[i] = newDef;
    //         ESP_LOGI("MacroDeviceDefinitionDataModel", "Reloaded definition: #%s", id.c_str());
    //         return true;
    //     }
    // }
    // // New definition — append
    // definitions.push_back(newDef);
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Added new definition: #%s", id.c_str());
    xSemaphoreGive(arrayMutex);
    return true;
}

MacroDeviceDefinition *MacroDeviceDefinitionDataModel::GetMacroDeviceDefinition(const char *id) {
    xSemaphoreTake(arrayMutex, portMAX_DELAY);
    for(int i=0; i<MaxMacroDeviceDefinitions; i++) {
        if (strcmp(definitions[i].id, id) == 0) {
            xSemaphoreGive(arrayMutex);
            return &definitions[i];
        }
    }

    xSemaphoreGive(arrayMutex);
    return nullptr;
}

void MacroDeviceDefinitionDataModel::SerializeListJSON(std::string *output) {
    xSemaphoreTake(arrayMutex, portMAX_DELAY);
    Document d;
    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(int i=0; i<MaxMacroDeviceDefinitions; i++) {
        if (definitions[i].id[0] == '\0') continue;
        d["machines"].PushBack(Value(definitions[i].id, d.GetAllocator()), d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    // ESP_LOGD("MacroDeviceDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
    xSemaphoreGive(arrayMutex);
}

bool MacroDeviceDefinitionDataModel::UpdateDefinition(const std::string &jsonString) {
    Document d;

    if (d.Parse(jsonString.c_str()).HasParseError()) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to parse JSON string: %s", jsonString.c_str());
        return false;
    }

    if (!d.HasMember("id")) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "JSON string does not contain 'id' field: %s", jsonString.c_str());
        return false;
    }

    const std::string id = d["id"].GetString();

    // Write to /user/macros/ (user data layer)

    const std::string path = CTAG::STORAGE::userPath() + "/" + CTAG::STORAGE::DIR_MACROS;
    const std::string filename = path + "/" + id + ".json";

    fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "could not open file %s", filename.c_str());
        return false;
    }
    fwrite(jsonString.c_str(), 1, jsonString.size(), fp);
    fclose(fp);

    // ReloadMachineDefinitions();

    return true;
}


void MacroDeviceDefinitionDataModel::SerializeItemJSON(const std::string &id, std::string *output) {
    // Overlay: check /user/macros/ then /factory/macros/
    const std::string filename = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_MACROS, id + ".json");
    if (filename.empty()) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Macro def not found in overlay: %s", id.c_str());
        output->assign("");
        return;
    }

    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == NULL) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "could not open file %s", filename.c_str());
        output->assign("");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    ESP_LOGI("MacroDeviceDefinitionDataModel", "File size %ld bytes", filesize);

    taskYIELD();

    char *content = (char *) heap_caps_malloc(filesize + 1, MALLOC_CAP_SPIRAM);
    if (content == nullptr) {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to allocate memory for reading macro device definition file");
        output->assign("");
        fclose(fp);
        return;
    }
    fread(content, 1, filesize, fp);
    fclose(fp);

    content[filesize] = '\0';

    // ESP_LOGD("MacroDeviceDefinitionDataModel", "JSON string %s", content);

    output->assign(content);

    heap_caps_free(content);
}


void MacroDeviceDefinitionDataModel::DeleteItem(const std::string &id) {
    // Only delete from /user/macros/ (factory macros are immutable)
    const std::string filename = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_MACROS, id + ".json");
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Deleting file: %s", filename.c_str());

    unlink(filename.c_str());
}

MacroDeviceDefinitionDataModel &MacroDeviceDefinitionDataModel::instance() {
    static MacroDeviceDefinitionDataModel instance;
    return instance;
}