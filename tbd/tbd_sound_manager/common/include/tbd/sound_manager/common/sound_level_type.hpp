#pragma once

#include <concepts>
#include <cinttypes>


namespace tbd::audio {

template<class SoundLevelT>
concept SoundLevelType = requires(SoundLevelT sound_level, uint32_t _uint32_t) {
    // deviation from std::mutex: lock timeout
    { sound_level.add_input_level(_uint32_t) } -> std::same_as<void>;
    { sound_level.add_output_level(_uint32_t) } -> std::same_as<void>;
};

}