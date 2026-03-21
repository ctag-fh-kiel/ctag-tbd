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

MacroDeviceOutputMappingSource::MacroDeviceOutputMappingSource() {
    parameterIndex = 0;
    multiplier = 1;
    divider = 1;
    curve = MacroCurveType::Linear;
}

MacroDeviceOutputMappingSource::~MacroDeviceOutputMappingSource() {}

bool MacroDeviceOutputMappingSource:: DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("src") && jsonelement["src"].IsUint()) {
        parameterIndex = jsonelement["src"].GetUint();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'src' field");
        return false;
    }

    if (jsonelement.HasMember("mul") && jsonelement["mul"].IsInt()) {
        multiplier = jsonelement["mul"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'mul' field");
        return false;
    }

    if (jsonelement.HasMember("div") && jsonelement["div"].IsInt()) {
        divider = jsonelement["div"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMappingSource", "Missing or invalid 'div' field");
        return false;
    }

    // Parse optional curve type (defaults to Linear if missing)
    curve = MacroCurveType::Linear;
    if (jsonelement.HasMember("curve") && jsonelement["curve"].IsString()) {
        const char *curveStr = jsonelement["curve"].GetString();
        if (strcmp(curveStr, "log") == 0) {
            curve = MacroCurveType::Log;
        } else if (strcmp(curveStr, "exp") == 0) {
            curve = MacroCurveType::Exp;
        }
        // "linear" or unknown → stays Linear
    }

    return true;
}

MacroDeviceOutputMapping::MacroDeviceOutputMapping() {
    ctrl = 0;
    startValue = 0;
    sources.clear();
}

MacroDeviceOutputMapping::~MacroDeviceOutputMapping() {}

bool MacroDeviceOutputMapping::DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceOutputMapping", "Not a JSON object");
        return false;
    }

    if (jsonelement.HasMember("ctrl") && jsonelement["ctrl"].IsInt()) {
        ctrl = jsonelement["ctrl"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'ctrl' field");
        return false;
    }

    if (jsonelement.HasMember("start") && jsonelement["start"].IsInt()) {
        startValue = jsonelement["start"].GetInt();
    } else {
        ESP_LOGE("MacroDeviceOutputMapping", "Missing or invalid 'start' field");
        return false;
    }

    if (jsonelement.HasMember("add") && jsonelement["add"].IsArray()) {
        for (auto &v : jsonelement["add"].GetArray()) {
            MacroDeviceOutputMappingSource s;
            if (s.DeserializeJSON(v)) {
                sources.push_back(s);
            } else {
                ESP_LOGE("MacroDeviceOutputMapping", "Failed to deserialize a source in 'add' array");
            }
        }
    }

    return true;
}

MacroDeviceDefinition::MacroDeviceDefinition() {
    id = "";
    name = "";
    synthId = "";
    volumeMultiplier = 1.0f;
    outputMappings.clear();
}

MacroDeviceDefinition::~MacroDeviceDefinition() {
}

bool MacroDeviceDefinition::DeserializeJSON(const rapidjson::Value &jsonelement) {
    if (!jsonelement.IsObject()) {
        ESP_LOGE("MacroDeviceDefinition", "Not a JSON object");
        return false;
    }

     if (jsonelement.HasMember("id") && jsonelement["id"].IsString()) {
        id = jsonelement["id"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'id' field");
        return false;
    }

    if (jsonelement.HasMember("name") && jsonelement["name"].IsString()) {
        name = jsonelement["name"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'name' field");
        return false;
    }

    if (jsonelement.HasMember("machine") && jsonelement["machine"].IsString()) {
        synthId = jsonelement["machine"].GetString();
    } else {
        ESP_LOGE("MacroDeviceDefinition", "Missing or invalid 'machine' field");
        return false;
    }

    if (jsonelement.HasMember("volmult") && jsonelement["volmult"].IsNumber()) {
        volumeMultiplier = jsonelement["volmult"].GetFloat();
    } else {
        volumeMultiplier = 1.0f;
    }

    if (jsonelement.HasMember("mapping") && jsonelement["mapping"].IsArray()) {
        for (auto &v : jsonelement["mapping"].GetArray()) {
            MacroDeviceOutputMapping m;
            if (m.DeserializeJSON(v)) {
                outputMappings.push_back(m);
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

MacroDeviceDefinition *MacroDeviceDefinition::copy() {
    MacroDeviceDefinition *newCopy = new MacroDeviceDefinition();
    newCopy->id = this->id;
    newCopy->name = this->name;
    newCopy->synthId = this->synthId;
    newCopy->volumeMultiplier = this->volumeMultiplier;
    newCopy->outputMappings = this->outputMappings;
    return newCopy;
}
