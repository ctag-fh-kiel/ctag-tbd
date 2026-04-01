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

#include "MacroDeviceDefinition.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <cstring>


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;

void MacroDeviceOutputMappingSource_Reset(struct MacroDeviceOutputMappingSource *source) {
    source->parameterIndex = 0;
    source->multiplier = 0;
    source->divider = 1;
    source->curve = MacroCurveType::Linear;
}

bool MacroDeviceOutputMappingSource_DeserializeJSON(struct MacroDeviceOutputMappingSource *source, const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("src") && jsonelement["src"].IsUint()) {
        source->parameterIndex = jsonelement["src"].GetUint();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'src' field");
        return false;
    }

    if (jsonelement.HasMember("mul") && jsonelement["mul"].IsInt()) {
        source->multiplier = jsonelement["mul"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'mul' field");
        return false;
    }

    if (jsonelement.HasMember("div") && jsonelement["div"].IsInt()) {
        source->divider = jsonelement["div"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'div' field");
        return false;
    }

    // Parse optional curve type (defaults to Linear if missing)
    source->curve = MacroCurveType::Linear;
    if (jsonelement.HasMember("curve") && jsonelement["curve"].IsString()) {
        const char *curveStr = jsonelement["curve"].GetString();
        if (strcmp(curveStr, "log") == 0) {
            source->curve = MacroCurveType::Log;
        } else if (strcmp(curveStr, "exp") == 0) {
            source->curve = MacroCurveType::Exp;
        }
        // "linear" or unknown → stays Linear
    }

    return true;
}

void MacroDeviceOutputMapping_Reset(struct MacroDeviceOutputMapping *mapping) {
    mapping->ctrl = 0;
    mapping->startValue = 0;
    for(int i=0; i<MaxOutputMappingSources; i++) {
        MacroDeviceOutputMappingSource_Reset(&mapping->sources[i]);
    }
}

bool MacroDeviceOutputMapping_DeserializeJSON(struct MacroDeviceOutputMapping *mapping, const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMapping", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("ctrl") && jsonelement["ctrl"].IsInt()) {
        mapping->ctrl = jsonelement["ctrl"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'ctrl' field");
        return false;
    }

    if (jsonelement.HasMember("start") && jsonelement["start"].IsInt()) {
        mapping->startValue = jsonelement["start"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'start' field");
        return false;
    }

    int index = 0;
    if (jsonelement.HasMember("add") && jsonelement["add"].IsArray()) {
        for (auto &v : jsonelement["add"].GetArray()) {
            if (!MacroDeviceOutputMappingSource_DeserializeJSON(&mapping->sources[index], v)) {
                ESP_LOGE("MacroDeviceOutputMapping", "Failed to deserialize a source in 'add' array");
            }
            index ++;
        }
    }

    return true;
}

void MacroDeviceDefinitionUtils::MacroDeviceDefinition_Reset(struct MacroDeviceDefinition *def) {
    memset(def->id, 0, sizeof(def->id));
    memset(def->name, 0, sizeof(def->name));
    memset(def->synthId, 0, sizeof(def->synthId));
    def->volumeMultiplier = 1.0f;
    for(int i=0; i<MaxOutputMappings; i++) {
        MacroDeviceOutputMapping_Reset(&def->outputMappings[i]);
    }
}

bool MacroDeviceDefinitionUtils::MacroDeviceDefinition_DeserializeJSON(struct MacroDeviceDefinition *def, const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceDefinition", "Not a JSON object");
        return false;
    }

     if (jsonelement.HasMember("id") && jsonelement["id"].IsString()) {
        strncpy(def->id, jsonelement["id"].GetString(), sizeof(def->id) - 1);
        def->id[sizeof(def->id) - 1] = '\0';
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'id' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        strncpy(def->name, jsonelement["name"].GetString(), sizeof(def->name) - 1);
        def->name[sizeof(def->name) - 1] = '\0';
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("machine") && jsonelement["machine"].IsString()) {
        strncpy(def->synthId, jsonelement["machine"].GetString(), sizeof(def->synthId) - 1);
        def->synthId[sizeof(def->synthId) - 1] = '\0';
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'machine' field");
        return false;
    }

    if (jsonelement.HasMember("volmult") && jsonelement["volmult"].IsNumber()) {
        def->volumeMultiplier = jsonelement["volmult"].GetFloat();
    } else {
        def->volumeMultiplier = 1.0f;
    }

    int index = 0;
    if (jsonelement.HasMember("mapping") && jsonelement["mapping"].IsArray()) {
        for (auto &v : jsonelement["mapping"].GetArray()) {
            if (MacroDeviceOutputMapping_DeserializeJSON(&def->outputMappings[index], v)) {
                index ++;
            } else {
                ESP_LOGE("MacroDeviceDefinition", "Failed to deserialize an output mapping in 'mapping' array");
            }
        }
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'mapping' field");
        return false;
    }

    return true;
}

void MacroDeviceDefinitionUtils::MacroDeviceDefinition_CopyInto(const struct MacroDeviceDefinition *source, struct MacroDeviceDefinition *target) {
    strncpy(target->id, source->id, sizeof(target->id) - 1);
    target->id[sizeof(target->id) - 1] = '\0';
    strncpy(target->name, source->name, sizeof(target->name) - 1);
    target->name[sizeof(target->name) - 1] = '\0';
    strncpy(target->synthId, source->synthId, sizeof(target->synthId) - 1);
    target->synthId[sizeof(target->synthId) - 1] = '\0';
    target->volumeMultiplier = source->volumeMultiplier;
    for(int i=0; i<MaxOutputMappings; i++) {
        memcpy(&target->outputMappings[i], &source->outputMappings[i], sizeof(struct MacroDeviceOutputMapping));
    }
}
