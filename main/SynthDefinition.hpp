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
        enum SynthParameterType {
            SynthParameterType_None = 0,
            SynthParameterType_CC = 1,
            SynthParameterType_NRPM = 2,
        };

        class SynthParameter {
            public:
                std::string id;
                std::string name;
                enum SynthParameterType type;
                uint16_t defaultValue;
                uint8_t cc;

            public:
                SynthParameter();
                ~SynthParameter();
        };

        enum SynthType {
            SynthType_None = 0,
            SynthType_Synth = 1,
            SynthType_Drum = 2,
        };

        class SynthDefinition {
            public:
                std::string id;
                std::string name;
                enum SynthType type;
                std::vector<SynthParameter*> parameters;

            public:
                SynthDefinition();
                ~SynthDefinition();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // bool SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator);
        };
    }
}