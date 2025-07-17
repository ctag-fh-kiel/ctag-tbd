#pragma once

#include <tbd/audio_device/concepts/audio_consumer_type.hpp>
#include <tbd/sound_processor/channels.hpp>
#include <tbd/sound_processor.hpp>
#include <tbd/errors.hpp>
#include <tbd/system/cpu_cores.hpp>

#include <utility>

namespace tbd::audio {

template<class AudioWorkerTypeT>
concept AudioWorkerType = requires(
    AudioWorkerTypeT audio_worker,
    bool _bool,
    AudioWorkerTypeT::params_type _params,
    sound_processor::channels::ChannelID channel,
    sound_processor::channels::Channels channels,
    sound_processor::SoundProcessor* sound_processor)
{
    { audio_worker.get_sound_processor(channel) } -> std::same_as<sound_processor::SoundProcessor*>;
    { audio_worker.set_sound_processor(channels, sound_processor) } -> std::same_as<Error>;
    { audio_worker.reset_sound_processor(channels) } -> std::same_as<Error>;

    { audio_worker.init(std::move(_params)) } -> std::same_as<uint32_t>;

    // begin just as task module
    { audio_worker.begin(_bool) } -> std::same_as<uint32_t>;
    { audio_worker.begin() } -> std::same_as<uint32_t>;
    // end just as task module
    { audio_worker.end(_bool) } -> std::same_as<uint32_t>;
    { audio_worker.end() } -> std::same_as<uint32_t>;

};

}
