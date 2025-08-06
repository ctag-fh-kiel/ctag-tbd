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


#include <cstdio>
#include <string>
#include <fstream>
#include "SPManagerDataModel.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <dirent.h>
#include "esp_log.h"
#include "ctagResources.hpp"

using namespace CTAG::AUDIO;


SPManagerDataModel::SPManagerDataModel() {
    ESP_LOGI("SPModel", "Trying to read config file");
    loadJSON(m, MODELJSONFN);
    getSoundProcessors();
    validateActiveProcessors();
    validatePatches();
}

SPManagerDataModel::~SPManagerDataModel() {
}

// checks for available sound processors based on data/sp json file entries
void SPManagerDataModel::getSoundProcessors() {
    if (m.HasMember("availableProcessors")) return;
    DIR *dir;
    struct dirent *ent;
    Value sparray(kArrayType);
    m.AddMember("availableProcessors", sparray, m.GetAllocator());
    if ((dir = opendir(string(CTAG::RESOURCES::spiffsRoot + string("/data/sp")).c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            string fn(ent->d_name);
            if (fn.find("mui-") != string::npos) {
                ESP_LOGD("SPModel", "Filename: %s", fn.c_str());
                Document d;
                loadJSON(d, CTAG::RESOURCES::spiffsRoot + "/data/sp/" + fn);
                Value obj(kObjectType);
                Value id(d["id"].GetString(), d.GetAllocator());
                Value name(d["name"].GetString(), d.GetAllocator());
                Value hint(kStringType);
                obj.AddMember("id", id.Move(), m.GetAllocator());
                obj.AddMember("name", name.Move(), m.GetAllocator());
                obj.AddMember("isStereo", d["isStereo"], m.GetAllocator());
                if (d.HasMember("hint")) {
                    hint.SetString(d["hint"].GetString(), m.GetAllocator());
                    obj.AddMember("hint", hint.Move(), m.GetAllocator());
                }
                m["availableProcessors"].PushBack(obj, m.GetAllocator());
            }
        }
        closedir(dir);
    }
    storeJSON(m, MODELJSONFN);
}

const char *SPManagerDataModel::GetCStrJSONSoundProcessors() {
    loadJSON(m, MODELJSONFN);
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.HasMember("availableProcessors")) return nullptr;
    m["availableProcessors"].Accept(writer);
    return json.GetString();
}

string SPManagerDataModel::GetActiveProcessorID(const int chan) {
    if (chan > 1 || chan < 0) return string("");
    if (!m.HasMember("activeProcessors")) return string();
    if (!m["activeProcessors"].IsArray()) return string();
    if (m["activeProcessors"].GetArray().Size() == 0) return string();
    return m["activeProcessors"].GetArray()[chan].GetString();
}

void SPManagerDataModel::SetActivePluginID(const string &id, const int chan) {
    if (chan > 1 || chan < 0) return;
    if (!m.HasMember("activeProcessors")) return;
    if (!m["activeProcessors"].IsArray()) return;
    if (m["activeProcessors"].Size() == 0) return;
    m["activeProcessors"][chan].SetString(id, m.GetAllocator());
    storeJSON(m, MODELJSONFN);
}

void SPManagerDataModel::SetActivePatchNum(const int patchNum, const int chan) {
    if (chan > 1 || chan < 0) return;
    string id = GetActiveProcessorID(chan);
    if (!m.HasMember("lastPatches")) return;
    if (!m["lastPatches"].IsArray()) return;
    if (m["lastPatches"].GetArray().Size() <= chan) return;
    if (!m["lastPatches"][chan].IsArray()) return;
    for (auto &chanPatches : m["lastPatches"][chan].GetArray()) {
        if (!chanPatches.HasMember("id")) return;
        if (!chanPatches["id"].IsString()) return;
        if (chanPatches["id"].GetString() == id) {
            chanPatches["patchNumber"] = patchNum;
            break;
        }
    }
    storeJSON(m, MODELJSONFN);
}

int SPManagerDataModel::GetActivePatchNum(const int chan) {
    if (chan > 1 || chan < 0) return 0;
    string id = GetActiveProcessorID(chan);
    if (!m.HasMember("lastPatches")) return 0;
    if (!m["lastPatches"].IsArray()) return 0;
    if (m["lastPatches"].GetArray().Size() <= chan) return 0;
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

bool SPManagerDataModel::IsStereo(const string &id) {
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
    if (!m.HasMember("lastPatches")) return;
    if (!m["lastPatches"].IsArray()) return;
    for (auto &chanPatches : m["lastPatches"].GetArray()) {
        //ESP_LOGD("SPModel", "chanP %d, avaiP %d", chanPatches.Size(), m["availableProcessors"].GetArray().Size());
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
    storeJSON(m, MODELJSONFN);
}

void SPManagerDataModel::validateActiveProcessors() {
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
    storeJSON(m, MODELJSONFN);
}

void SPManagerDataModel::PrintSelf() {
    printJSON(m);
}

const char *SPManagerDataModel::GetCStrJSONConfiguration() {
    if (!m.HasMember("configuration")) return nullptr;
    json.Clear();
    Writer<StringBuffer> writer(json);
    m["configuration"].Accept(writer);
    return json.GetString();
}

void SPManagerDataModel::SetConfigurationFromJSON(const string &data) {
    if (!m.HasMember("configuration")) return;
    Document d;
    d.Parse(data);
    if(d.HasParseError()) return;
    Value obj(kObjectType);
    obj.CopyFrom(d, m.GetAllocator());
    m["configuration"] = obj.Move();
    storeJSON(m, MODELJSONFN);
    //PrintSelf();
}

string SPManagerDataModel::GetConfigurationData(const string &id) {
    //PrintSelf();
    if (!m.HasMember("configuration")) return string();
    Value s(kStringType);
    s.CopyFrom(m["configuration"][id], m.GetAllocator());
    return s.GetString();
}

string SPManagerDataModel::GetNetworkConfigurationData(const string &which) {
    if (!m.HasMember("configuration")) return string();
    if (!m["configuration"].HasMember("wifi")) return string();
    Value s(kStringType);
    s.CopyFrom(m["configuration"]["wifi"][which], m.GetAllocator());
    return s.GetString();
}

void SPManagerDataModel::ResetNetworkConfiguration() {
    if (!m.HasMember("configuration")) return;
    if (!m["configuration"].HasMember("wifi")) return;
    Value ssid("ctag-tbd");
    Value pwd("");
    Value mode("ap");
    m["configuration"]["wifi"]["ssid"] = ssid.Move();
    m["configuration"]["wifi"]["pwd"] = pwd.Move();
    m["configuration"]["wifi"]["mode"] = mode.Move();
    storeJSON(m, MODELJSONFN);
}

const char *SPManagerDataModel::GetCStrJSONSoundProcessorPresets(const string &id) {
    json.Clear();
    Document d1, d2;
    loadJSON(d1, CTAG::RESOURCES::spiffsRoot + "/data/sp/mp-" + id + ".jsn");
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
    ESP_LOGD("Model", "String %s", data);
    Document presets;
    presets.Parse(data);
    if(presets.HasParseError()) return;
    storeJSON(presets, string(CTAG::RESOURCES::spiffsRoot + "/data/sp/mp-" + id + ".jsn"));
}

bool SPManagerDataModel::HasPluginID(const string &id) {
    if (!m.HasMember("availableProcessors")) return false;
    for(auto &v: m["availableProcessors"].GetArray()){
        if(v.HasMember("id")){
            if(v["id"].GetString() == id) return true;
        }
    }
    return false;
}
