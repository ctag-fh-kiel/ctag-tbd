#pragma once

#include <tbd/errors.hpp>

TBD_NEW_ERR(SOUND_REGISTRY_INVALID_INPUT_ID, "invalid input id")
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_PLUGIN_ID, "invalid sound id")
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_PARAM_ID, "invalid parameter id")
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_CHANNEL_ID, "invalid channel id")
TBD_NEW_ERR(SOUND_REGISTRY_INVALID_RELATIVE_PARAM_ID, "invalid sound parameter id")
TBD_NEW_ERR(SOUND_REGISTRY_BAD_CHANNEL_MAPPING, "channel mapping is invalid value")
TBD_NEW_ERR(SOUND_REGISTRY_NEED_STEREO, "running in stereo mode, but input was mono mapping")

TBD_NEW_ERR(SOUND_REGISTRY_PARAM_NOT_INT, "can not assign int to parameter")
TBD_NEW_ERR(SOUND_REGISTRY_PARAM_NOT_UINT, "can not assign unsigned int to parameter")
TBD_NEW_ERR(SOUND_REGISTRY_PARAM_NOT_FLOAT, "can not assign float to parameter")
TBD_NEW_ERR(SOUND_REGISTRY_PARAM_NOT_TRIGGER, "can not assign bool to parameter")

TBD_NEW_ERR(SOUND_REGISTRY_PENDING_OPERATION, "previous sound processor update pending")
TBD_NEW_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT, "no plugin present on channel")
TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_IN_USE, "trying to emplace sound processor in channel that is in use")
TBD_NEW_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR, "sound processor pointer is null")
TBD_NEW_ERR(SOUND_REGISTRY_BAD_SOUND_PROCESSOR_ADDRESS, "sound processor pointer is at wrong address")
TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_NOT_RESERVED, "trying to emplace sound processor in channel without reserving memory")
