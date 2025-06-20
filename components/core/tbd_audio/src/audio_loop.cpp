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

SoundProcessor* set_sound_processor(const Channels channels, SoundProcessor* sound_processor) {
    return nullptr;
}

SoundProcessor* get_sound_processor(const Channels channels) {
    return nullptr;
}

}
