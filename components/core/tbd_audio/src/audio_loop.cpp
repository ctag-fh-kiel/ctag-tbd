#define TBD_IS_AUDIO_LOOP_COMPILATION_UNIT

#include <tbd/audio/audio_loop.hpp>
#include <tbd/audio/impl/audio_worker.hpp>

namespace tbd::audio_loop {

using namespace tbd::audio::channels;
using namespace tbd::sound_processor;

void begin() {
    audio::audio_worker.begin();

}

void end() {
    audio::audio_worker.end();
}

SoundProcessor* get_sound_processor(const ChannelID channel) {
    return audio::audio_worker.get_sound_processor(channel);
}

Error set_sound_processor(const Channels channels, SoundProcessor* sound_processor) {
    return audio::audio_worker.set_sound_processor(channels, sound_processor);
}

Error reset_sound_processor(const Channels channels) {
    return audio::audio_worker.reset_sound_processor(channels);
}

}
