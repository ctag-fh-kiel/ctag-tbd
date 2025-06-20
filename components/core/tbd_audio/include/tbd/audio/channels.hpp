#pragma once

namespace tbd::audio::channels {

enum Channel {
    CH_0       = 0,
    CH_1       = 1,
    CH_INVALID = 2,
};

enum Channels {
    CM_NONE    = 0,
    CM_LEFT    = 1 << CH_0,
    CM_RIGHT   = 1 << CH_1,
    CM_BOTH    = CH_0 | CH_1,
    CM_INVALID = CM_BOTH + 1,
};

}
