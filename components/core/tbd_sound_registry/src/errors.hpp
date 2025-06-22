#include <tbd/errors.hpp>

TBD_NEW_ERR(SOUND_REGISTRY_INVALID_INPUT_ID, "invalid input id");
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_PLUGIN_ID, "invalid sound id");
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_PARAM_ID, "invalid parameter id");
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_RELATIVE_PARAM_ID, "invalid sound parameter id");
TBD_NEW_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT, "no plugin present on channel");
TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_IN_USE, "trying to emplace sound processor in channel that is in use");
TBD_NEW_ERR(SOUND_REGISTRY_BAD_CHANNEL_MAPPING, "channel mapping is invalid value");
TBD_NEW_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR, "sound processor pointer is null");
TBD_NEW_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR_ADDRESS, "sound processor pointer is at wrong address");
TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_IN_USE, "channel in use");
TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_NOT_RESERVED, "trying to emplace sound processor in channel without reserving memory");
