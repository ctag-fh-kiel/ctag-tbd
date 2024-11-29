#pragma once

#include <concepts>


namespace tbd::audio {

template<class AudioConsumerT>
concept AudioConsumerType = requires(
    AudioConsumerT audio_consumer, 
    uint32_t _uint32_t, 
    float _float,
    float* _float_ptr)
{
    { audio_consumer.startup() } -> std::same_as<uint32_t>;
    { audio_consumer.consume(_float_ptr) } -> std::same_as<uint32_t>;
    { audio_consumer.cleanup() } -> std::same_as<uint32_t>;
};

}
