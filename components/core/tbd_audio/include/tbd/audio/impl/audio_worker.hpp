#pragma once

#include <tbd/audio/concepts/audio_worker_type.hpp>
#include <tbd/audio/impl/audio_consumer.hpp>

#if TBD_AUDIO_PULL
    #include <tbd/audio/impl/pull_audio_worker.hpp>

    namespace tbd::audio {

    using AudioParams = PullAudioParams;
    using AudioWorker = PullAudioWorker<system::CpuCore::audio>;

    }

#elif TBD_AUDIO_PUSH
    #include <tbd/audio_device/impl/push_audio_worker.hpp>

    namespace tbd::audio {

    using AudioParams = PushAudioParams;
    using AudioWorker = PushAudioWorker;

    }
#else
    #error "neither pull nor push audio IO is selected: TBD_AUDIO_PULL or TBD_AUDIO_PUSH need to be set"
#endif

namespace tbd::audio {

static_assert(AudioWorkerType<AudioWorker>, "audio worker type does not satisfy requirements");

AudioWorker audio_worker("audio_worker");

}

