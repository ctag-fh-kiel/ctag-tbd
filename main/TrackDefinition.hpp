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
        const int MaxTrackDefinitionMachineIds = 8;

        struct TrackDefinition {
            int index;
            char name[16];
            int midiChannel;
            int drumNote;
            int baseCC;
            char macroMachineIds[MaxTrackDefinitionMachineIds][16];
        };

        class TrackDefinitionUtils final {
        public:
            TrackDefinitionUtils() = delete;

            static void TrackDefinition_Reset(struct TrackDefinition *def);
            static bool TrackDefinition_DeserializeJSON(struct TrackDefinition *def, const rapidjson::Value &jsonelement);
        };
    }
}