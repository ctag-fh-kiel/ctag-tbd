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


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


SynthParameter::SynthParameter() {
    id = "";
    name = "";
    type = SynthParameterType_None;
    defaultValue = 0;
    cc = 0;
}

SynthParameter::~SynthParameter() {
}






SynthDefinition::SynthDefinition() {
    id = "";
    name = "";
    type = SynthType_None;
    parameters.clear();
}

SynthDefinition::~SynthDefinition() {
}

bool SynthDefinition::DeserializeJSON(const Value &jsonelement) {
    if (!jsonelement.HasMember("id")) return false;
    if (!jsonelement["id"].IsString()) return false;
    id = jsonelement["id"].GetString();

    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement["name"].IsString()) return false;
    name = jsonelement["name"].GetString();

    if (!jsonelement.HasMember("type")) return false;
    if (!jsonelement["type"].IsString()) return false;

    std::string typestring = jsonelement["name"].GetString();
    type = SynthType_None;
    if (typestring == "synth") {
        type = SynthType_Synth;
    }
    if (typestring == "drum") {
        type = SynthType_Drum;
    }
    // type = static_cast<SynthType>(jsonelement["type"].GetInt());

    ESP_LOGI("SynthDefinition", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    parameters.clear();
    if (jsonelement.HasMember("parameters") && jsonelement["parameters"].IsArray()) {
        for (auto &v : jsonelement["parameters"].GetArray()) {
            if (!v.HasMember("id")) return false;
            if (!v.HasMember("name")) return false;
            if (!v.HasMember("type")) return false;
            if (!v.HasMember("def")) return false;
            if (!v.HasMember("ctrl")) return false;

            SynthParameter *p = new SynthParameter();
            p->id = v["id"].GetString();
            p->name = v["name"].GetString();
            if (v["type"].GetString() == std::string("cc")) {
                p->type = SynthParameterType_CC;
            } else if (v["type"].GetString() == std::string("nrpm")) {
                p->type = SynthParameterType_NRPM;
            } else {
                p->type = SynthParameterType_None;
            }
            p->defaultValue = v["def"].GetUint();
            p->cc = v["ctrl"].GetUint();

            parameters.push_back(p);
        }
    }

    return true;
}

// bool SynthDefinition::SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator) {
//     // Implementation goes here

//     // Value obj(kObjectType);
//     // Value id(jsonelement["id"].GetString(), allocator);
//     // Value name(jsonelement["name"].GetString(), allocator);
//     // Value hint(kStringType);

//     // jsonelement.AddMember("id", id.Move(), allocator);
//     // jsonelement.AddMember("name", name.Move(), allocator);
//     // // jsonelement.PushBack(obj.Move(), allocator);

//     return true;
// }
