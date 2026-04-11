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

#include "SynthDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void SynthParameter_Reset(struct SynthParameter *param) {
    memset(param->id, 0, sizeof(param->id));
    memset(param->name, 0, sizeof(param->name));
    param->type = SynthParameterType_None;
    param->defaultValue = 0;
    param->cc = 0;
}


void CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_Reset(struct SynthDefinition *def) {
    memset(def->id, 0, sizeof(def->id));
    memset(def->name, 0, sizeof(def->name));
    def->type = SynthType_None;
    for(int pi=0; pi<MaxSynthDefinitionParameters; pi++) {
        SynthParameter_Reset(&def->parameters[pi]);
    }
}

bool CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_DeserializeJSON(struct SynthDefinition *def, const Value &jsonelement) {
    if (!jsonelement.HasMember("id")) return false;
    if (!jsonelement["id"].IsString()) return false;
    strncpy(def->id, jsonelement["id"].GetString(), sizeof(def->id) - 1);

    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement["name"].IsString()) return false;
    strncpy(def->name, jsonelement["name"].GetString(), sizeof(def->name) - 1);

    if (!jsonelement.HasMember("type")) return false;
    if (!jsonelement["type"].IsString()) return false;

    std::string typestring = jsonelement["type"].GetString();
    def->type = SynthType_None;
    if (typestring == "synth") {
        def->type = SynthType_Synth;
    }
    if (typestring == "drum") {
        def->type = SynthType_Drum;
    }

    int index = 0;
    if (jsonelement.HasMember("parameters") && jsonelement["parameters"].IsArray()) {
        for (auto &v : jsonelement["parameters"].GetArray()) {
            if (!v.HasMember("id")) return false;
            if (!v.HasMember("name")) return false;
            if (!v.HasMember("type")) return false;
            if (!v.HasMember("def")) return false;
            if (!v.HasMember("ctrl")) return false;

            SynthParameter *p = &def->parameters[index];
            strncpy(p->id, v["id"].GetString(), sizeof(p->id) - 1);
            strncpy(p->name, v["name"].GetString(), sizeof(p->name) - 1);
            std::string typestring = v["type"].GetString();
            if (typestring == "cc") {
                p->type = SynthParameterType_CC;
            } else if (typestring == "nrpm") {
                p->type = SynthParameterType_NRPM;
            } else {
                p->type = SynthParameterType_None;
            }
            p->defaultValue = v["def"].GetUint();
            p->cc = v["ctrl"].GetUint();
            index ++;
        }
    }

    return true;
}

bool CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_SerializeJSON(struct SynthDefinition *def, std::string *jsonoutput) {
    *jsonoutput = "{\"dummy\":42}";



    Document d;

    d.SetObject();

    d.AddMember("id", Value(def->id, d.GetAllocator()), d.GetAllocator());
    d.AddMember("name", Value(def->name, d.GetAllocator()), d.GetAllocator());

    Value paramsarray(kArrayType);
    d.AddMember("parameters", paramsarray, d.GetAllocator());
    for(int i=0; i<MaxSynthDefinitionParameters; i++) {
        if (def->parameters[i].id[0] == '\0') {
            continue;
        }

        Value param(kObjectType);
        param.AddMember("id", Value(def->parameters[i].id, d.GetAllocator()), d.GetAllocator());
        param.AddMember("name", Value(def->parameters[i].name, d.GetAllocator()), d.GetAllocator());
        std::string typestring;
        if (def->parameters[i].type == SynthParameterType_CC) {
            typestring = "cc";
        } else if (def->parameters[i].type == SynthParameterType_NRPM) {
            typestring = "nrpm";
        } else {
            typestring = "none";
        }
        param.AddMember("type", Value(typestring.c_str(), d.GetAllocator()), d.GetAllocator());
        param.AddMember("ctrl", def->parameters[i].cc, d.GetAllocator());
        param.AddMember("def", def->parameters[i].defaultValue, d.GetAllocator());

        d["parameters"].PushBack(param, d.GetAllocator());
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    ESP_LOGD("SynthDefinitionUtils", "JSON string %s", buffer.GetString());

    jsonoutput->assign(buffer.GetString());

    return true;
}