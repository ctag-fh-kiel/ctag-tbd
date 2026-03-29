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

#include "MacroSoundPreset.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "ctagDataModelBase.hpp"

using namespace CTAG::MACROPRESETS;
using namespace rapidjson;

void MacroSoundPresetUtils::MacroSoundPreset_Reset(struct MacroSoundPreset *preset) {
    memset(preset->id, 0, sizeof(preset->id));
    memset(preset->displayName, 0, sizeof(preset->displayName));
    memset(preset->groupName, 0, sizeof(preset->groupName));
    memset(preset->macroDeviceId, 0, sizeof(preset->macroDeviceId));
    preset->validTracksBitmask = 0;
    for(int k=0; k<MaxMacroSoundPresetParameters; k++) {
        preset->parameterValues[k] = 0;
    }
}

bool MacroSoundPresetUtils::MacroSoundPreset_DeserializeJSON(struct MacroSoundPreset *preset, const rapidjson::Value &jsonelement) {
    // Implement deserialization logic here

    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroSoundPreset", "Not a JSON object");
        return false;
    }

     if (jsonelement.HasMember("id") && jsonelement["id"].IsString()) {
        strncpy(preset->id, jsonelement["id"].GetString(), sizeof(preset->id) - 1);
        preset->id[sizeof(preset->id) - 1] = '\0';
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'id' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        strncpy(preset->displayName, jsonelement["name"].GetString(), sizeof(preset->displayName) - 1);
        preset->displayName[sizeof(preset->displayName) - 1] = '\0';
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("group") && jsonelement["group"].IsString()) {
        strncpy(preset->groupName, jsonelement["group"].GetString(), sizeof(preset->groupName) - 1);
        preset->groupName[sizeof(preset->groupName) - 1] = '\0';
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'group' field");
        return false;
    }

    if (jsonelement.HasMember("macro") && jsonelement["macro"].IsString()) {
        strncpy(preset->macroDeviceId, jsonelement["macro"].GetString(), sizeof(preset->macroDeviceId) - 1);
        preset->macroDeviceId[sizeof(preset->macroDeviceId) - 1] = '\0';
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'macro' field");
        return false;
    }

    if (jsonelement.HasMember("values") && jsonelement["values"].IsArray()) {
        int index = 0;
        for (auto &v : jsonelement["values"].GetArray()) {
            if (v.IsInt()) {
                if (index < MaxMacroSoundPresetParameters) {
                    preset->parameterValues[index] = v.GetInt();
                }
                index++;
            } else {
                ESP_LOGE("MacroSoundPreset", "Invalid 'values' array element");
                return false;
            }
        }
    } else {
        ESP_LOGE("MacroSoundPreset", "Missing or invalid 'values' field");
        return false;
    }

    return true;
}

void MacroSoundPresetUtils::MacroSoundPresetGroup_Reset(struct MacroSoundPresetGroup *group) {
    memset(group->id, 0, sizeof(group->id));
    memset(group->displayName, 0, sizeof(group->displayName));
    group->validTracksBitmask = 0;
    group->numFileIds = 0;
    memset(group->fileIds, 0, sizeof(group->fileIds));
}
