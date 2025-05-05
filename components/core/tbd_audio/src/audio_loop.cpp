#define TBD_IS_AUDIO_LOOP_COMPILATION_UNIT

#include <tbd/audio_loop/audio_loop.hpp>
#include <tbd/audio_loop/impl/audio_worker.hpp>


namespace tbd::audio_loop {

void begin() {
    audio::audio_worker.begin();

}

void end() {
    audio::audio_worker.end();
}

audio::SoundProcessor* set_sound_processor(uint8_t audio_channel, audio::SoundProcessor* sound_processor) {
    return nullptr;
}

audio::SoundProcessor* get_sound_processor(uint8_t audio_channel) {
    return nullptr;
}

}
