#pragma once

#include <tbd/sound_registry/plugin_meta_base.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>

#include <tbd/audio/audio_loop.hpp>
#include <tbd/sound_processor/channels.hpp>
#include <tbd/parameter_types.hpp>
#include <tbd/maybe.hpp>
#include <concepts>


namespace tbd::sound_registry {

namespace channels = sound_processor::channels;


[[tbd::event]]
void sound_processor_changed(const uint_par& channels, const uint_par& new_plugin_id);

[[tbd::event]]
void sound_processor_param_changed(const uint_par& channels, const uint_par& param_id, const uint_par& new_value);

[[tbd::event]]
void sound_processor_mapping_changed(const uint_par& channels, const uint_par& param_id, const uint_par& new_cv);

[[tbd::event]]
void sound_processors_reset();


struct ActiveSoundProcessors final {
    ActiveSoundProcessors() = delete;

    [[nodiscard]] static bool is_stereo();
    [[nodiscard]] static Maybe<parameters::PluginID> get_processor_on_channels(channels::Channels channels);

    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, int_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, uint_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, float_par value);
    static Error set_param(channels::Channels channels, parameters::ParameterID param_id, trigger_par value);
    static Error map_param(channels::Channels channels, parameters::ParameterID param_id, parameters::InputID input_id);

    /** Create a new sound processor and assign to channel(s).
     *
     */
    template<std::derived_from<PluginMetaBase> PluginT>
    static Error set_plugin(const channels::Channels channels) {
        if (const auto err = audio_loop::reset_sound_processor(channels); err) {
            return err;
        }

        auto plugin_memory = reserve_plugin_memory(channels, sizeof(PluginT));
        if (!plugin_memory) {
            return plugin_memory.error();
        }
        PluginMetaBase* plugin = new (plugin_memory.value()) PluginT;
        if (const auto err = set_active_plugin(channels, plugin); err != TBD_OK) {
            return err;
        }
        plugin->init();
        if (const auto err = audio_loop::set_sound_processor(channels, plugin); err != TBD_OK) {
            return err;
        }
        sound_processor_changed(channels, plugin->id());
        return TBD_OK;
    }

    /** Delete all sound processors.
     *
     */
    static void reset();

    [[nodiscard]] static size_t remaining_buffer_size();

private:
    static Maybe<void*> reserve_plugin_memory(channels::Channels channels, size_t plugin_size);
    static Error set_active_plugin(channels::Channels channels, PluginMetaBase* plugin);
    static Maybe<void*> reserve_chunk(channels::Channels channels, size_t plugin_size);
};

}
