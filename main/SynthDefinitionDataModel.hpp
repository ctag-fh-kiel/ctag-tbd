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
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class TrackDefinition;
        class SynthDefinition;

        class SynthDefinitionDataModel final : public CTAG::SP::ctagDataModelBase {
            private:
                std::vector<TrackDefinition*> tracks;
                std::vector<SynthDefinition*> synths;

            public:
                SynthDefinitionDataModel();
                ~SynthDefinitionDataModel();
                void ReloadSynthDefinitions();
                int GetNumberOfSynthDefinitions();
                void GetSynthDeviceDefinitionId(int index, std::string *idOutput);
                void GetSynthDefinitionsJSON(const std::string *output);
                SynthDefinition *GetSynthDefinition(const std::string id);
                TrackDefinition *GetTrackDefinition(int index);
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                void SerializeJSON(std::string *output);
                void SerializeTrackJSON(int index, std::string *output);
                void SerializeSynthJSON(const std::string id, std::string *output);
                void SerializeListJSON(std::string *output);
        };
    }
}