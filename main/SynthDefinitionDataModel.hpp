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
#include "SpiApiProtocol.h"
#include "ctagDataModelBase.hpp"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"


#define MAX_TRACKS 20
#define MAX_SYNTHS 32

namespace CTAG {
    namespace MACROPRESETS {

        class SynthDefinitionDataModel final : public CTAG::SP::ctagDataModelBase {
            private:
                class SynthDefinition *synths;
                class TrackDefinition *tracks;

            public:
                // SynthDefinitionDataModel(const SynthDefinitionDataModel&) = delete;

                void Init();
                void ReloadSynthDefinitions();
                int GetNumberOfSynthDefinitions();
                void GetSynthDeviceDefinitionId(int index, std::string *idOutput);
                void GetSynthDefinitionsJSON(const std::string *output);
                SynthDefinition *GetSynthDefinition(const std::string id);
                TrackDefinition *GetTrackDefinition(int index);
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // void SerializeJSON(std::string *output);
                void SerializeListJSON(std::string *output);
                void WriteListResponse(struct GetEngineDefinitionIdListResponse *response);

                static SynthDefinitionDataModel *instance();
        };
    }
}