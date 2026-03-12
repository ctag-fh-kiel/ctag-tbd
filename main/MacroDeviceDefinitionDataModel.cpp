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


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


MacroDeviceDefinitionDataModel::MacroDeviceDefinitionDataModel() {
}

MacroDeviceDefinitionDataModel::~MacroDeviceDefinitionDataModel() {
}

void MacroDeviceDefinitionDataModel::ReloadMachineDefinitions() {
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Trying to read macro device definition file");

    Document d;
    for(MacroDeviceDefinition *def : definitions) {
        delete def;
    }
    definitions.clear();

    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    // m.AddMember("availableProcessors", sparray, m.GetAllocator());
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fn(ent->d_name);

            // ESP_LOGI("MacroDeviceDefinitionDataModel", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
            //          heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
            //          heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
            //          heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
            //          heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

            Document d;
            loadJSON(d, path + "/" + fn);
            if(!d.HasParseError()) {
                MacroDeviceDefinition *def = new MacroDeviceDefinition();
                if(def->DeserializeJSON(d)) {
                    ESP_LOGI("MacroDeviceDefinitionDataModel", "Deserialized macro device definition: #%s \"%s\"", def->id.c_str(), def->name.c_str());
                    definitions.push_back(def);
                } else {
                    ESP_LOGE("MacroDeviceDefinitionDataModel", "Failed to deserialize macro device definition from file %s", fn.c_str());
                    delete def;
                }
            } else {
                ESP_LOGI("MacroDeviceDefinitionDataModel", "Failed to parse file: %s", fn.c_str());

            }
        }
        closedir(dir);
    } else {
        ESP_LOGE("MacroDeviceDefinitionDataModel", "Could not open directory %s", path.c_str());
    }

}

MacroDeviceDefinition *MacroDeviceDefinitionDataModel::LoadMacroDeviceDefinition(std::string id) {
    for(MacroDeviceDefinition *def : definitions) {
        if (def->id == id) {
            return def->copy();
        }
    }

    return nullptr;
}

void MacroDeviceDefinitionDataModel::SerializeListJSON(std::string *output) {
    Document d;
    d.SetObject();

    Value machinesarray(kArrayType);
    d.AddMember("machines", machinesarray, d.GetAllocator());
    for(MacroDeviceDefinition *s : definitions) {
        d["machines"].PushBack(Value(s->id.c_str(), d.GetAllocator()), d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    // ESP_LOGD("MacroDeviceDefinitionDataModel", "JSON string %s", buffer.GetString());

    output->assign(buffer.GetString());
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

    std::string id = d["id"].GetString();

    // just save the file now when we know the id

    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));
    std::string filename = path + "/" + id + ".json";

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
    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));
    std::string filename = path + "/" + id + ".json";

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
    // TODO: Just delete from disk?

    std::string path = std::string(CTAG::RESOURCES::sdcardRoot + std::string("/data/macrodefinitions"));

    std::string filename = path + "/" + id + ".json";
    ESP_LOGI("MacroDeviceDefinitionDataModel", "Deleting file: %s", filename.c_str());

    unlink(filename.c_str());
}
