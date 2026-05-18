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
#include "SharedEngineDefinition.hpp"
#include "SharedTrackDefinition.hpp"
#include "ctagDataModelBase.hpp"


#define MAX_TRACKS 20
#define MAX_SYNTHS 32

namespace CTAG {
    namespace MACROPRESETS {

        class SynthDefinitionDataModel final : public CTAG::SP::ctagDataModelBase {
            private:
                struct SharedEngineDefinition *synths;
                struct SharedTrackDefinition *tracks;

                int lastTrack;
                int lastEngine;

                void addDrumTrack(const char *name, int midiChannel, int baseCC, int drumNote, const char *defaultbank);
                void addSynthTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank);
                void addFxTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank);
                void addTrackEngine(const char *machineId);

                void addEngine(const char *id, const char *name, enum SharedEngineType type);
                void addEngineParameter(const char *paramId, const char *name, enum SharedEngineParameterType ctrltype, int ctrl, int defaultValue);

            public:
                void Init();
                int GetNumberOfSynthDefinitions();
                struct SharedEngineDefinition *GetSynthDefinition(const std::string id);
                struct SharedTrackDefinition *GetTrackDefinition(int index);
                bool SerializeIntoJSON(rapidjson::Document &doc);

                void WriteListResponse(struct GetEngineDefinitionIdListResponse *response);
                void WriteEngineDefinitionPageResponse(const struct GetEngineDefinitionsPageRequest *request, struct GetEngineDefinitionsPageResponse *response);

                static SynthDefinitionDataModel *instance();
        };
    }
}