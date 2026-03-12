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

#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        class TrackDefinition {
            public:
                int index;
                std::string name;
                int midiChannel;
                int drumNote;
                int baseCC;
                std::string activeMachineId;
                std::vector<std::string> macroMachineIds;
                // std::vector<std::string> macroPresetIds;

            public:
                TrackDefinition();
                ~TrackDefinition();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator);
        };
    }
}