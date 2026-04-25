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
        const int MaxOutputMappingSources = 8;
        // Widened 2026-04-24 from 16 → 20 to fit `ro-basic` (18 mappings) and
        // `td3-acidbass` (17 mappings) without silent UB overflow in the
        // deserializer. Net cost: +544 bytes per MacroDeviceDefinition
        // instance on P4 SPIRAM — negligible. Validator + WebUI caps updated
        // to match. See macro-system.md "Structural caps".
        const int MaxOutputMappings = 20;

        // Response curve types for parameter mapping
        enum class MacroCurveType : uint8_t {
            Linear = 0,   // Default: straight 1:1 mapping
            Log    = 1,   // Logarithmic: for frequency/cutoff (pitch is logarithmic)
            Exp    = 2,   // Exponential: for decay/envelope times (resolution for short times)
        };

        enum CtrlType : uint8_t {
            CtrlType_CC = 1,
            CtrlType_NRPM = 2,
        };

        struct MacroDeviceOutputMappingSource {
            uint8_t parameterIndex;
            int32_t multiplier;
            int32_t divider;
            MacroCurveType curve;
        };

        struct MacroDeviceOutputMapping {
            int8_t ctrl;
            enum CtrlType ctrltype;
            int16_t startValue;
            struct MacroDeviceOutputMappingSource sources[MaxOutputMappingSources];
        };

        struct MacroDeviceDefinition {
            char id[32];
            char name[32];
            char synthId[32];
            float volumeMultiplier;
            struct MacroDeviceOutputMapping outputMappings[MaxOutputMappings];
        };

        class MacroDeviceDefinitionUtils final {
            public:
            MacroDeviceDefinitionUtils() = delete;

            static void MacroDeviceDefinition_CopyInto(const struct MacroDeviceDefinition *source, struct MacroDeviceDefinition *target);
            static void MacroDeviceDefinition_Reset(struct MacroDeviceDefinition *def);
            static bool MacroDeviceDefinition_DeserializeJSON(struct MacroDeviceDefinition *def, const rapidjson::Value &jsonelement);
        };
    }
}