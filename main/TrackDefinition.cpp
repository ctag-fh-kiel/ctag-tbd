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

#include "TrackDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


TrackDefinition::TrackDefinition() {
    index = 0;
    name = "";
    midiChannel = 0;
    drumNote = 0;
    baseCC = 0;
    macroMachineIds.clear();
}

TrackDefinition::~TrackDefinition() {
}

bool TrackDefinition::DeserializeJSON(const Value &jsonelement) {
    if (!jsonelement.HasMember("index")) return false;
    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement.HasMember("midichannel")) return false;
    if (!jsonelement.HasMember("drumnote")) return false;
    if (!jsonelement.HasMember("basecc")) return false;

    index = jsonelement["index"].GetInt();
    name = jsonelement["name"].GetString();
    midiChannel = jsonelement["midichannel"].GetInt();
    drumNote = jsonelement["drumnote"].GetInt();
    baseCC = jsonelement["basecc"].GetInt();

    ESP_LOGI("TrackDefinition", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
   
    macroMachineIds.clear();
    if (jsonelement.HasMember("machines") && jsonelement["machines"].IsArray()) {
        for (auto &v : jsonelement["machines"].GetArray()) {
            macroMachineIds.push_back(v.GetString());
        }
    }

    return true;
}

bool TrackDefinition::SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator) {
    // Implementation goes here
    // parentjsonelement
    return true;
}

