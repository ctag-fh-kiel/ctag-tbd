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
        const int MaxSynthDefinitionParameters = 24;

        enum SynthParameterType : uint8_t {
            SynthParameterType_None = 0,
            SynthParameterType_CC = 1,
            SynthParameterType_NRPM = 2,
        };

        enum SynthType : uint8_t {
            SynthType_None = 0,
            SynthType_Synth = 1,
            SynthType_Drum = 2,
            SynthType_FX = 3,
        };

        struct SynthParameter {
            char id[16];
            char name[32];
            enum SynthParameterType type;
            uint16_t defaultValue;
            uint8_t cc;
        };

        struct SynthDefinition {
            char id[16];
            char name[32];
            enum SynthType type;
            struct SynthParameter parameters[MaxSynthDefinitionParameters];
        };

        class SynthDefinitionUtils final {
        public:
            SynthDefinitionUtils() = delete;

            static void SynthDefinition_Reset(struct SynthDefinition *def);
            // static bool SynthDefinition_DeserializeJSON(struct SynthDefinition *def, const rapidjson::Value &jsonelement);
            // static bool SynthDefinition_SerializeJSON(struct SynthDefinition *def, std::string *jsonoutput);
        };
    }
}