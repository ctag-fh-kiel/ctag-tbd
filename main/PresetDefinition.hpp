#pragma once

#include <stdint.h>

const int MaxPresetIdLength = 16;
const int MaxPresetsPerPage = 8;
const int MaxPresetParameters = 32;

//
// sound presets (macro presets)
//

struct MacroSoundPreset2 {
    char id[MaxPresetIdLength];
    char displayName[16];
    char groupName[16];
    char macroDefinitionId[MaxPresetIdLength];
    int32_t parameterValues[MaxPresetParameters];
};

struct GetMacroSoundPresetsPageRequest {
    uint16_t offset;
};

struct GetMacroSoundPresetsPageResponse {
    uint16_t offset;
    uint16_t totalMacroPresets;
    uint16_t returnedMacroPresets;
    struct MacroSoundPreset2 macroPresets[MaxPresetsPerPage];
};

struct ApplyMacroSoundPresetRequest {
    uint8_t trackIndex;
    char id[MaxPresetIdLength];
};
