#pragma once

#include <stdint.h>

const int MaxTrackDefinitionEngineIds = 8;
const int MaxTrackDefinitionEngineIdLength = 16;

enum TrackType : uint8_t {
    TRACK_TYPE_UNKNOWN = 0,
    TRACK_TYPE_DRUM = 1,
    TRACK_TYPE_SYNTH = 2,
    TRACK_TYPE_FX = 3,
    TRACK_TYPE_AUDIOINPUT = 4,
};

struct TrackDefinition {
    uint8_t index;
    enum TrackType type;
    int8_t midiChannel;
    int8_t drumNote;
    int8_t baseCC;
    char engineIdStr[MaxTrackDefinitionEngineIds][MaxTrackDefinitionEngineIdLength];
    char name[16];
};
