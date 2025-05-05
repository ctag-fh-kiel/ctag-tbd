#pragma once

#include <tbd/sound_processor.hpp>

namespace tbd::audio_loop {
    /** @brief Set a new sound processor for channel.
     *  
     *  Ownership remains with the caller.
     * 
     *  @return the previous sound processor on said channel
     */
    audio::SoundProcessor* set_sound_processor(uint8_t audio_channel, audio::SoundProcessor* sound_processor);

    /** @brief Get the current sound processor for channel.
     * 
     */
    audio::SoundProcessor* get_sound_processor(uint8_t audio_channel);

    void begin();
    void end();
}