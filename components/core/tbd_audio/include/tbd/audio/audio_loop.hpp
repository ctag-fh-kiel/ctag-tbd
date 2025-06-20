#pragma once

#include <tbd/sound_processor.hpp>
#include <tbd/audio/channels.hpp>

namespace tbd::audio_loop {

void begin();
void end();

sound_processor::SoundProcessor* set_sound_processor(audio::channels::Channels, sound_processor::SoundProcessor* sound_processor);
sound_processor::SoundProcessor* get_sound_processor(audio::channels::Channels audio_channel);

}
