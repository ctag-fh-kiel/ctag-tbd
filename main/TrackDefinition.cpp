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


void CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_Reset(struct TrackDefinition *def) {
    def->index = 0;
    memset(def->name, 0, sizeof(def->name));
    def->midiChannel = 0;
    def->drumNote = 0;
    def->baseCC = 0;
    memset(def->macroMachineIds, 0, sizeof(def->macroMachineIds));
}

bool CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_DeserializeJSON(struct TrackDefinition *def, const Value &jsonelement) {
    if (!jsonelement.HasMember("index")) return false;
    if (!jsonelement.HasMember("name")) return false;
    if (!jsonelement.HasMember("midichannel")) return false;
    if (!jsonelement.HasMember("drumnote")) return false;
    if (!jsonelement.HasMember("basecc")) return false;

    def->index = jsonelement["index"].GetInt();
    strncpy(def->name, jsonelement["name"].GetString(), sizeof(def->name) - 1);
    def->name[sizeof(def->name) - 1] = '\0';
    def->midiChannel = jsonelement["midichannel"].GetInt();
    def->drumNote = jsonelement["drumnote"].GetInt();
    def->baseCC = jsonelement["basecc"].GetInt();

    memset(def->macroMachineIds, 0, sizeof(def->macroMachineIds));
    if (jsonelement.HasMember("machines") && jsonelement["machines"].IsArray()) {
        int i = 0;
        for (auto &v : jsonelement["machines"].GetArray()) {
            if (i < MaxTrackDefinitionMachineIds) {
                strncpy(def->macroMachineIds[i], v.GetString(), sizeof(def->macroMachineIds[i]) - 1);
                def->macroMachineIds[i][sizeof(def->macroMachineIds[i]) - 1] = '\0';
                i++;
            }
        }
    }

    return true;
}
