#pragma once

#include <tbd/audio_device/concepts/audio_consumer_type.hpp>
#include <tbd/system/cpu_cores.hpp>

#include <utility>

namespace tbd::audio {

template<template<AudioConsumerType AudioConsumerT, system::CpuCore> class AudioWorkerTypeT>
concept AudioWorkerType = requires(
    AudioWorkerTypeT<VoidAudioConsumer, system::CpuCore::audio> audio_worker,
    bool _bool,
    AudioWorkerTypeT<VoidAudioConsumer, system::CpuCore::audio>::params_type _params)
{
    { audio_worker.init(std::move(_params)) } -> std::same_as<uint32_t>;

    // begin just as task module
    { audio_worker.begin(_bool) } -> std::same_as<uint32_t>;
    { audio_worker.begin() } -> std::same_as<uint32_t>;
    // end just as task module
    { audio_worker.end(_bool) } -> std::same_as<uint32_t>;
    { audio_worker.end() } -> std::same_as<uint32_t>;

};

}
