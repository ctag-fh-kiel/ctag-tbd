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

#include <vector>
#include <set>
#include <string>
#include <stdint.h>
#include <iostream>
#include <memory>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        const int MaxMacroSoundPresetParameters = 24;
        const int MaxPresetsPerGroup = 64;

        struct MacroSoundPreset {
            char id[32];
            char displayName[32];
            char groupName[32];
            char macroDeviceId[32];
            uint32_t validTracksBitmask;
            int32_t parameterValues[MaxMacroSoundPresetParameters];
        };

        struct MacroSoundPresetGroup {
            char id[32];
            char displayName[32];
            uint32_t validTracksBitmask;
            uint8_t numFileIds;
            char fileIds[MaxPresetsPerGroup][32];
        };

        class MacroSoundPresetUtils final {
            public:
            MacroSoundPresetUtils() = delete;

            static bool MacroSoundPreset_DeserializeJSON(struct MacroSoundPreset *preset, const rapidjson::Value &jsonelement);
            static void MacroSoundPreset_Reset(struct MacroSoundPreset *preset);

            static void MacroSoundPresetGroup_Reset(struct MacroSoundPresetGroup *group);
        };
    }
}