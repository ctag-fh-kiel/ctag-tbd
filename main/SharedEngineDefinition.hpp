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

#include <stdint.h>

//
// synth/machine definitions are fetched ONCE per boot, shouldn't have to be cached.
//

const int MaxEngineDefinitionParameters = 24;
const int MaxEngineDefinitionsPerPage = 8;
const int MaxEngines = 32;
const int MaxEngineId = 16;
const int MaxEngineDefinitionsPerPage = 8;

enum SharedEngineParameterType : uint8_t {
    EngineParameterType_None = 0,
    EngineParameterType_CC = 1,
    EngineParameterType_NRPM = 2,
    EngineParameterType_Hidden = 3,
};

struct SharedEngineParameter {
    int8_t index;
    enum SharedEngineParameterType type;
    uint16_t defaultValue;
    uint8_t relCC;
    char name[32];
};

struct SharedEngineDefinition {
    uint32_t id; // fourcc
    char idStr[MaxEngineId]; // string version
    char name[8]; // string version
    struct SharedEngineParameter parameters[MaxEngineDefinitionParameters];
};

struct GetEngineDefinitionsPageRequest {
    uint16_t offset;
};

struct GetEngineDefinitionsPageResponse {
    uint16_t offset;
    uint16_t totalEngineDefinitions;
    uint16_t returnedEngineDefinitions;
    struct SharedEngineDefinition engineDefinitions[MaxEngineDefinitionsPerPage];
};

struct GetEngineDefinitionIdListResponse {
    uint16_t numEngines;
    char engineIds[MaxEngines][MaxEngineId];
};
