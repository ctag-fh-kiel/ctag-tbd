#pragma once

#include <cinttypes>

namespace tbd::sound_processor::channels {

enum ChannelID {
    CH_0       = 0,
    CH_1       = 1,
    CH_INVALID = 2,
};

inline ChannelID channel_id_from_int(const uint32_t value) {
    if (value == CH_0) {
        return CH_0;
    }
    if (value == CH_1) {
        return CH_1;
    }
    return CH_INVALID;
}

enum Channels {
    CM_NONE    = 0,
    CM_LEFT    = 1 << CH_0,
    CM_RIGHT   = 1 << CH_1,
    CM_BOTH    = CM_LEFT | CM_RIGHT,
    CM_INVALID = CM_BOTH + 1,
};

inline Channels channels_from_int(const uint32_t value) {
    if (value == CM_NONE) {
        return CM_NONE;
    }
    if (value == CM_LEFT) {
        return CM_LEFT;
    }
    if (value == CM_RIGHT) {
        return CM_RIGHT;
    }
    if (value == CM_BOTH) {
        return CM_BOTH;
    }
    return CM_INVALID;
}

}
