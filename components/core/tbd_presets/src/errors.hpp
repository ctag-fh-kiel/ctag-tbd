#pragma once

#include <tbd/errors.hpp>

TBD_NEW_ERR(PRESETS_NO_PRESET_PRESENT, "no preset preset active on channel")
TBD_NEW_ERR(PRESETS_NEEDS_STEREO, "running in stereo mode, but input was mono mapping")
TBD_NEW_ERR(PRESETS_BAD_CHANNEL_MAPPING, "channel mapping is invalid value")
