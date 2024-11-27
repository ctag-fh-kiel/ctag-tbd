/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/
#include <tbd/sound_manager/data_model.hpp>

#include <cstdio>
#include <fstream>
#include <format>
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <tbd/storage/resources.hpp>
#include <tbd/logging.hpp>
#include <tbd/storage/fileystem.hpp>


using namespace rapidjson;
namespace fs = tbd::storage::filesystem;


namespace tbd::audio {

SPManagerDataModel::SPManagerDataModel() {
    TBD_LOGV("sound_manager", "loading sound manager config file");

    auto config_path = storage::get_fs_path("data/spm-config.jsn");
    if (!config_path) {
        TBD_LOGE("sound_manager", "failed to load sound manager config");
        return;
    }
    _config_file = config_path->string();
    loadJSON(m, *_config_file);

    getSoundProcessors();
    validateActiveProcessors();
    validatePatches();
}

SPManagerDataModel::~SPManagerDataModel() {

}

// checks for available sound processors based on data/sp json file entries
void SPManagerDataModel::getSoundProcessors() {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (m.HasMember("availableProcessors")) return;
    Value sparray(kArrayType);
    m.AddMember("availableProcessors", sparray, m.GetAllocator());

    auto sound_processor_configs_path = storage::get_fs_path("data/sp");
    if (!sound_processor_configs_path || !fs::is_directory(*sound_processor_configs_path)) {
        TBD_LOGE("sound_manager", "invalid sound processor config dir");
        return;
    }

    for (auto& plugin_description : fs::directory_iterator(*sound_processor_configs_path)) {
        auto& file_path = plugin_description.path();
        if (file_path.filename().string().rfind("mui-", 0) == 0) {
            TBD_LOGW("sound_manager", "file: %s", file_path.filename().c_str());
        }
    }

    // FIXME: not implemented
    // if ((dir = opendir(std::string(CTAG::RESOURCES::spiffsRoot + std::string("/data/sp")).c_str())) != NULL) {
    //     while ((ent = readdir(dir)) != NULL) {
    //         std::string fn(ent->d_name);
    //         if (fn.find("mui-") != std::string::npos) {
    //             TBD_LOGD("SPModel", "Filename: %s", fn.c_str());
    //             Document d;
    //             loadJSON(d, CTAG::RESOURCES::spiffsRoot + "/data/sp/" + fn);
    //             Value obj(kObjectType);
    //             Value id(d["id"].GetString(), d.GetAllocator());
    //             Value name(d["name"].GetString(), d.GetAllocator());
    //             Value hint(kStringType);
    //             obj.AddMember("id", id.Move(), m.GetAllocator());
    //             obj.AddMember("name", name.Move(), m.GetAllocator());
    //             obj.AddMember("isStereo", d["isStereo"], m.GetAllocator());
    //             if (d.HasMember("hint")) {
    //                 hint.SetString(d["hint"].GetString(), m.GetAllocator());
    //                 obj.AddMember("hint", hint.Move(), m.GetAllocator());
    //             }
    //             m["availableProcessors"].PushBack(obj, m.GetAllocator());
    //         }
    //     }
    //     closedir(dir);
    // }
    // storeJSON(m, MODELJSONFN);
}

const char *SPManagerDataModel::GetCStrJSONSoundProcessors() {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return "";
    }

    loadJSON(m, *_config_file);
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.HasMember("availableProcessors")) return nullptr;
    m["availableProcessors"].Accept(writer);
    return json.GetString();
}

std::string SPManagerDataModel::GetActiveProcessorID(const int chan) {
    if (chan > 1 || chan < 0) return {};
    if (!m.HasMember("activeProcessors")) return {};
    if (!m["activeProcessors"].IsArray()) return {};
    if (m["activeProcessors"].GetArray().Size() == 0) return {};
    return m["activeProcessors"].GetArray()[chan].GetString();
}

void SPManagerDataModel::SetActivePluginID(const std::string &id, const int chan) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (chan > 1 || chan < 0) return;
    if (!m.HasMember("activeProcessors")) return;
    if (!m["activeProcessors"].IsArray()) return;
    if (m["activeProcessors"].Size() == 0) return;
    m["activeProcessors"][chan].SetString(id, m.GetAllocator());
    storeJSON(m, *_config_file);
}

void SPManagerDataModel::SetActivePatchNum(const int patchNum, const int chan) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (chan > 1 || chan < 0) return;
    std::string id = GetActiveProcessorID(chan);
    if (!m.HasMember("lastPatches")) return;
    if (!m["lastPatches"].IsArray()) return;
    if (!m["lastPatches"][chan].IsArray()) return;
    for (auto &chanPatches : m["lastPatches"][chan].GetArray()) {
        if (!chanPatches.HasMember("id")) return;
        if (!chanPatches["id"].IsString()) return;
        if (chanPatches["id"].GetString() == id) {
            chanPatches["patchNumber"] = patchNum;
            break;
        }
    }
    storeJSON(m, *_config_file);
}

int SPManagerDataModel::GetActivePatchNum(const int chan) {
    if (chan > 1 || chan < 0) return 0;
    std::string id = GetActiveProcessorID(chan);
    if (!m.HasMember("lastPatches")) return 0;
    if (!m["lastPatches"].IsArray()) return 0;
    if (!m["lastPatches"][chan].IsArray()) return 0;
    for (auto &chanPatches : m["lastPatches"][chan].GetArray()) {
        if (!chanPatches.HasMember("id")) return 0;
        if (!chanPatches["id"].IsString()) return 0;
        if (chanPatches["id"].GetString() == id) {
            if (!chanPatches.HasMember("patchNumber")) return 0;
            if (!chanPatches["patchNumber"].IsInt()) return 0;
            return chanPatches["patchNumber"].GetInt();
        }
    }
    return 0;
}

bool SPManagerDataModel::IsStereo(const std::string &id) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return false;
    }

    if (!m.HasMember("availableProcessors")) return false;
    for(auto &v: m["availableProcessors"].GetArray()){
        if(v.HasMember("id")){
            if(id.compare(v["id"].GetString()) == 0){
                return v["isStereo"].GetBool();
            }
        }
    }
    return false;
}

void SPManagerDataModel::validatePatches() {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (!m.HasMember("lastPatches")) return;
    if (!m["lastPatches"].IsArray()) return;
    for (auto &chanPatches : m["lastPatches"].GetArray()) {
        //TBD_LOGD("SPModel", "chanP %d, avaiP %d", chanPatches.Size(), m["availableProcessors"].GetArray().Size());
        if (!m.HasMember("availableProcessors")) return;
        if (!m["availableProcessors"].IsArray()) return;
        if (chanPatches.Size() != m["availableProcessors"].GetArray().Size()) {
            for (auto &processor : m["availableProcessors"].GetArray()) {
                Value obj(kObjectType);
                Value s(processor["id"].GetString(), m.GetAllocator());
                obj.AddMember("id", s.Move(), m.GetAllocator());
                obj.AddMember("patchNumber", 0, m.GetAllocator());
                chanPatches.PushBack(obj, m.GetAllocator());
            }
        }
    }
    storeJSON(m, *_config_file);
}

void SPManagerDataModel::validateActiveProcessors() {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (!m.HasMember("activeProcessors")) return;
    if (m["activeProcessors"].Size() != 2) {
        for (auto &v: m["availableProcessors"].GetArray()) {
            if (!v.HasMember("isStereo")) return;
            if (!v["isStereo"].IsBool()) return;
            if (v["isStereo"].GetBool() == false) {
                Value id1(v["id"].GetString(), m.GetAllocator());
                Value id2(v["id"].GetString(), m.GetAllocator());
                m["activeProcessors"].PushBack(id1.Move(), m.GetAllocator());
                m["activeProcessors"].PushBack(id2.Move(), m.GetAllocator());
                break;
            }
        }
    }
    storeJSON(m, *_config_file);
}

void SPManagerDataModel::PrintSelf() {
    printJSON(m);
}

const char *SPManagerDataModel::GetCStrJSONConfiguration() {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return {};
    }

    if (!m.HasMember("configuration")) return nullptr;
    json.Clear();
    Writer<StringBuffer> writer(json);
    m["configuration"].Accept(writer);
    return json.GetString();
}

void SPManagerDataModel::SetConfigurationFromJSON(const std::string &data) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    if (!m.HasMember("configuration")) return;
    Document d;
    d.Parse(data);
    if(d.HasParseError()) return;
    Value obj(kObjectType);
    obj.CopyFrom(d, m.GetAllocator());
    m["configuration"] = obj.Move();
    storeJSON(m, *_config_file);
    //PrintSelf();
}

std::string SPManagerDataModel::GetConfigurationData(const std::string &id) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return {};
    }

    //PrintSelf();
    if (!m.HasMember("configuration")) return std::string();
    Value s(kStringType);
    s.CopyFrom(m["configuration"][id], m.GetAllocator());
    return s.GetString();
}

const char* SPManagerDataModel::GetCStrJSONSoundProcessorPresets(const std::string &id) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return "";
    }

    auto sound_processor_configs_dir = storage::get_fs_path("data/sp");
    if (!sound_processor_configs_dir || !fs::is_directory(*sound_processor_configs_dir)) {
        TBD_LOGE("sound_manager", "invalid sound processor config dir %s", sound_processor_configs_dir->c_str());
        return "";
    }

    const std::string sound_processor_presets_filename = std::format("mp-{}.jsn", id);
    auto sound_processor_presets_path = *sound_processor_configs_dir / sound_processor_presets_filename;
    if (!fs::is_regular_file(sound_processor_presets_path)) {
        TBD_LOGE("sound_manager", "invalid sound processor preset file %s", sound_processor_presets_path.c_str());
        return "";
    }

    json.Clear();
    Document d1, d2;
    loadJSON(d1, sound_processor_presets_path);
    d2.SetObject();
    Value s_id(kObjectType);
    s_id.SetString(id, d2.GetAllocator());
    d2.AddMember("id", s_id.Move(), d2.GetAllocator());
    d2.AddMember("presets", d1.Move(), d2.GetAllocator());
    Writer<StringBuffer> writer(json);
    d2.Accept(writer);
    return json.GetString();
}

void SPManagerDataModel::SetCStrJSONSoundProcessorPreset(const char *id, const char* data) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return;
    }

    auto sound_processor_configs_dir = storage::get_fs_path("data/sp");
    if (!sound_processor_configs_dir || !fs::is_directory(*sound_processor_configs_dir)) {
        TBD_LOGE("sound_manager", "invalid sound processor config dir %s", sound_processor_configs_dir->c_str());
        return;
    }

    auto sound_processor_presets_filename = std::format("mp-{}.jsn", id);
    auto sound_processor_presets_path = *sound_processor_configs_dir / sound_processor_presets_filename;
    if (!fs::is_regular_file(sound_processor_presets_path)) {
        TBD_LOGE("sound_manager", "invalid sound processor preset file %s", sound_processor_presets_path.c_str());
        return;
    }

    TBD_LOGD("Model", "String %s", data);
    Document presets;
    presets.Parse(data);
    if(presets.HasParseError()) return;
    storeJSON(presets, sound_processor_presets_path);
}

bool SPManagerDataModel::HasPluginID(const std::string &id) {
    if (_config_file) {
        TBD_LOGE("sound_manager", "access to broken sound manager config");
        return false;
    }

    if (!m.HasMember("availableProcessors")) return false;
    for(auto &v: m["availableProcessors"].GetArray()){
        if(v.HasMember("id")){
            if(v["id"].GetString() == id) return true;
        }
    }
    return false;
}

}