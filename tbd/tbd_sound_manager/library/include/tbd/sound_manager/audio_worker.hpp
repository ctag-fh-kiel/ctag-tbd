#pragma once

#include <tbd/system/modules.hpp>

#if TBD_AUDIO_PULL
    #include <tbd/sound_manager/common/pull_audio_feeder.hpp>

namespace tbd::audio {

using AudioParams = PullSoundParams;

template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = system::ModuleTask<PullAudioFeeder<AudioConsumerT>, cpu_core>;

}

#elif TBD_AUDIO_PUSH
    #include <tbd/sound_manager/port/push_audio_feeder.hpp>

namespace tbd::audio {

using AudioParams = PushAudioParams;

template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = PushAudioFeeder<AudioConsumerT>;

}
#else
    #error "neither pull nor push audio IO is selected: TBD_AUDIO_PULL or TBD_AUDIO_PUSH need to be set"
#endif
