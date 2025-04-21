#pragma once

#include <concepts>


namespace tbd::audio {

/** @brief echo consumer for testing purposes only
 *
 */
struct EchoAudioConsumer{
    uint32_t startup() { return 0; }
    uint32_t consume(float*) { return 0; }
    uint32_t cleanup() { return 0; }
};

template<class AudioConsumerT>
concept AudioConsumerType = requires(
    AudioConsumerT audio_consumer,
    float* _float_ptr)
{
    { audio_consumer.startup() } -> std::same_as<uint32_t>;
    { audio_consumer.consume(_float_ptr) } -> std::same_as<uint32_t>;
    { audio_consumer.cleanup() } -> std::same_as<uint32_t>;
};

}
