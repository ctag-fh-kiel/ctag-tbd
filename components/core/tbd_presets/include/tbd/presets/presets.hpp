#pragma once

#include <tbd/sound_processor/channels.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>
#include <tbd/maybe.hpp>

namespace tbd::presets {

using PresetID = uint16_t;

[[tbd::event]]
void preset_changed(const uint_par& channel_id, const uint_par& preset_id);

struct Presets {
    Presets() = delete;

    static Maybe<PresetID> get_active_preset(sound_processor::channels::Channels channels);
    static Error set_active_preset(sound_processor::channels::Channels channels, PresetID preset_id);

    static Maybe<size_t> get_num_presets_for_channel(sound_processor::channels::Channels channels);
    static Maybe<size_t> get_num_presets_for_plugin(sound_registry::parameters::PluginID plugin_id);
};

}
