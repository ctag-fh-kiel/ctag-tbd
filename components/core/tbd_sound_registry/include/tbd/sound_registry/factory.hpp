#pragma once

#include <tbd/sound_registry/active_sound_processors.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>

namespace tbd::sound_registry::factory {

template<class T>
Error activate_sound_processor(sound_processor::channels::Channels);

template<class T>
struct meta_for_sound_processor;

using Activator = Error (*)(sound_processor::channels::Channels);
extern Activator ACTIVATORS[];

inline Error set_plugin(const sound_processor::channels::Channels channels, const parameters::PluginID plugin_id) {
    if (plugin_id >= parameters::NUM_PLUGINS) {
        return TBD_ERR(INVALID_PLUGIN_ID);
    }
    return ACTIVATORS[plugin_id](channels);
}

};
