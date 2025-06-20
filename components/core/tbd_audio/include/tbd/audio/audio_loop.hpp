#pragma once

#include <tbd/audio/channels.hpp>
#include <tbd/sound_processor.hpp>
#include <tbd/errors.hpp>

namespace tbd::audio_loop {

void begin();
void end();

sound_processor::SoundProcessor* get_sound_processor(audio::channels::ChannelID channel);
Error set_sound_processor(audio::channels::Channels channels, sound_processor::SoundProcessor* sound_processor);
Error reset_sound_processor(audio::channels::Channels channels);

}
