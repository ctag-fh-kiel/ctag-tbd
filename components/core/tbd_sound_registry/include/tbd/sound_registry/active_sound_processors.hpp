#pragma once

#include <tbd/sound_registry/plugin_meta_base.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>

#include <tbd/errors.hpp>
#include <concepts>

TBD_NEW_ERR(SOUND_REGISTRY_CHANNEL_IN_USE, "trying to emplace sound processor in channel that is in use");
TBD_NEW_ERR(SOUND_REGISTRY_BAD_CHANNEL_MAPPING, "channel mapping is invalid value");

namespace tbd::sound_registry {

namespace channels {

enum ChannelIDs {
    CH0  = 0,
    CH1  = 1,
    NUM_CHANNELS = 2,
};

enum ChannelMapping {
    INVALID_MAPPING = 0,
    TO_LEFT   = 1 << CH0,
    TO_RIGHT  = 1 << CH1,
    TO_BOTH = TO_LEFT | TO_RIGHT,
};

inline ChannelMapping channel_mapping_from_int(const uint32_t value) {
    if (value == TO_LEFT) {
        return TO_LEFT;
    }
    if (value == TO_RIGHT) {
        return TO_RIGHT;
    }
    if (value == TO_BOTH) {
        return TO_BOTH;
    }
    return INVALID_MAPPING;
}

}

struct ActiveSoundProcessors final {
    ActiveSoundProcessors() = delete;

    [[nodiscard]] static bool is_stereo();
    [[nodiscard]] static parameters::PluginID on_left();
    [[nodiscard]] static parameters::PluginID on_right();

    /** Create a new sound processor and assign to channel(s).
     *
     */
    template<std::derived_from<PluginMetaBase> PluginT>
    static Error set_plugin(const channels::ChannelMapping channels) {
        auto [err, plugin_memory] = reserve_plugin_memory(channels, sizeof(PluginT));
        if (err != TBD_OK) {
            return err;
        }
        PluginMetaBase* plugin = new (plugin_memory) PluginT;
        if (err = set_active_plugin(channels, plugin); err != TBD_OK) {
            return err;
        }
        plugin->init();
        return TBD_OK;
    }

    /** Delete all sound processors.
     *
     */
    static void reset();

    [[nodiscard]] static size_t remaining_buffer_size();

private:
    static std::tuple<Error, void*> reserve_plugin_memory(channels::ChannelMapping channels, size_t plugin_size);
    static Error set_active_plugin(channels::ChannelMapping channels, PluginMetaBase* plugin);
    static std::tuple<Error, void*> reserve_chunk(channels::ChannelMapping channels, size_t plugin_size);
};

}
