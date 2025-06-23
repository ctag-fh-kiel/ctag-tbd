#pragma once

#include <tbd/sound_registry/plugin_meta_base.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>

#include <tbd/audio/audio_loop.hpp>
#include <tbd/sound_processor/channels.hpp>
#include <tbd/parameter_types.hpp>
#include <tuple>
#include <concepts>


namespace tbd::sound_registry {

namespace channels = sound_processor::channels;

struct ActiveSoundProcessors final {
    ActiveSoundProcessors() = delete;

    [[nodiscard]] static bool is_stereo();
    [[nodiscard]] static int16_t get_processor_on_channels(channels::Channels channels);

    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, int_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, uint_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, float_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, trigger_par value);

    /** Create a new sound processor and assign to channel(s).
     *
     */
    template<std::derived_from<PluginMetaBase> PluginT>
    static Error set_plugin(const sound_processor::channels::Channels channels) {
        if (const auto err = audio_loop::reset_sound_processor(channels); err) {
            return err;
        }

        auto [err, plugin_memory] = reserve_plugin_memory(channels, sizeof(PluginT));
        if (err != TBD_OK) {
            return err;
        }
        PluginMetaBase* plugin = new (plugin_memory) PluginT;
        if (err = set_active_plugin(channels, plugin); err != TBD_OK) {
            return err;
        }
        plugin->init();
        return audio_loop::set_sound_processor(channels, plugin);
    }

    /** Delete all sound processors.
     *
     */
    static void reset();

    [[nodiscard]] static size_t remaining_buffer_size();

private:
    static std::tuple<Error, void*> reserve_plugin_memory(sound_processor::channels::Channels channels, size_t plugin_size);
    static Error set_active_plugin(sound_processor::channels::Channels channels, PluginMetaBase* plugin);
    static std::tuple<Error, void*> reserve_chunk(sound_processor::channels::Channels channels, size_t plugin_size);
};

}
