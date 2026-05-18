#pragma once

#include <stdint.h>

//
// track definitions are fetched ONCE per boot, shouldn't have to be cached.
//

enum SharedTrackType : uint8_t {
    TRACK_TYPE_UNKNOWN = 0,
    TRACK_TYPE_DRUM = 1,
    TRACK_TYPE_SYNTH = 2,
    TRACK_TYPE_AUDIOINPUT = 3,
};

struct SharedTrackDefinition {
    uint8_t index;
    enum SharedTrackType type;
    int8_t midi_channel;
    int8_t drum_note;
    int8_t base_cc;
    uint8_t num_machine_ids;
    uint32_t machine_ids[16];
    char name[16];
};

struct GetTrackDefinitionsPageRequest {
    uint8_t offset;
};

struct GetTrackDefinitionsPageResponse {
    uint8_t offset;
    uint8_t totalTrackDefinitions;
    uint8_t returnedTrackDefinitions;
    struct SharedTrackDefinition trackDefinitions[8];
};
