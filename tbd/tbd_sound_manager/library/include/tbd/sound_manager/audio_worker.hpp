#pragma once

#include <tbd/system/task_module.hpp>
#include <tbd/sound_manager/common/audio_feeder_type.hpp>

#if TBD_AUDIO_PULL
    #include <tbd/sound_manager/common/pull_audio_worker.hpp>

namespace tbd::audio {

using AudioParams = PullAudioParams;

template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = PullAudioWorker<AudioConsumerT, cpu_core>;

}

#elif TBD_AUDIO_PUSH
    #include <tbd/sound_manager/port/push_audio_worker.hpp>

namespace tbd::audio {

using AudioParams = PushAudioParams;

template<AudioConsumerType AudioConsumerT, system::CpuCore cpu_core>
using AudioWorker = PushAudioWorker<AudioConsumerT>;

}
#else
    #error "neither pull nor push audio IO is selected: TBD_AUDIO_PULL or TBD_AUDIO_PUSH need to be set"
#endif

namespace tbd::audio {

static_assert(AudioWorkerType<AudioWorker>, "audio worker type does not satisfy requirements");

}