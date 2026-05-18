#pragma once

#include <stdint.h>

//
// macro definitions are fetched on every boot, and when web app has marked them as dirty.
//

const int MaxMacroOutputMappingSources = 8;
const int MaxMacroOutputMappings = 20;
const int MaxMacroIdLength = 16;
const int MaxMacrosPerPage = 8;

enum SharedMachineParameterType : uint8_t {
    MachineParameterType_None = 0,
    MachineParameterType_CC = 1,
    MachineParameterType_NRPM = 2,
};

enum SharedMachineType : uint8_t {
    MachineType_None = 0,
    MachineType_Synth = 1,
    MachineType_Drum = 2,
};

// Response curve types for parameter mapping
enum class SharedMacroCurveType : uint8_t {
    Linear = 0,   // Default: straight 1:1 mapping
    Log    = 1,   // Logarithmic: for frequency/cutoff (pitch is logarithmic)
    Exp    = 2,   // Exponential: for decay/envelope times (resolution for short times)
};

struct SharedMacroDefinitionOutputMappingSource {
    uint8_t parameterIndex;
    int32_t multiplier;
    int32_t divider;
    enum SharedMacroCurveType curve;
};

struct SharedMacroDefinitionOutputMapping {
    int8_t ctrl;
    enum SharedMachineParameterType ctrltype;
    int16_t startValue;
    struct SharedMacroDefinitionOutputMappingSource sources[MaxMacroOutputMappingSources];
};

struct SharedMacroDefinition {
    char id[MaxMacroIdLength];
    char displayName[16];
    uint32_t machineId;
    char machineIdStr[MaxMacroIdLength];
    uint16_t volumeMultiplier; // 1024 = 100%
    struct SharedMacroDefinitionOutputMapping outputMappings[MaxMacroOutputMappings];
};

struct GetMacroDefinitionsPageRequest {
    uint16_t offset;
};

struct GetMacroDefinitionsPageResponse {
    uint16_t offset;
    uint16_t totalMacroDefinitions;
    uint16_t returnedMacroDefinitions;
    struct SharedMacroDefinition macroDefinitions[MaxMacrosPerPage];
};
