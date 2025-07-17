#include <tbd/presets/presets.hpp>

#include "errors.hpp"
#include <tbd/sound_registry/active_sound_processors.hpp>


using namespace tbd::sound_processor::channels;
using namespace tbd::sound_registry::parameters;


namespace tbd::presets {

namespace {

struct PresetsImpl {
    Maybe<PresetID> get_active_preset(const Channels channels) const {
        if (sound_registry::ActiveSoundProcessors::is_stereo()) {
            if (channels != CM_BOTH) {
                return TBD_FAIL(PRESETS_NEEDS_STEREO);
            }
            return {static_cast<PresetID>(left_)};
        }
        if (channels == CM_LEFT) {
            return {static_cast<PresetID>(left_)};
        }
        if (channels == CM_RIGHT) {
            return {static_cast<PresetID>(right_)};
        }
        return TBD_FAIL(PRESETS_BAD_CHANNEL_MAPPING);
    }

    Error set_active_preset(const Channels channels, const PresetID preset_id) {
        const auto processor_id = sound_registry::ActiveSoundProcessors::get_processor_on_channels(channels);
        if (!processor_id) {
            return processor_id.error();
        }

        preset_changed(channels, preset_id);
        return TBD_OK;
    }

    Maybe<size_t> get_num_presets_for_channel(const PluginID plugin_id) {
        return TBD_OK;
    }

    Maybe<size_t> get_num_presets_for_plugin(const PluginID plugin_id) {
        return TBD_OK;
    }

    void reset_presets(const Channels channels) {
        if (channels & CM_LEFT) {
            left_ = -1;
        }
        if (channels & CM_RIGHT) {
            right_ = -1;
        }
    }

protected:
    int16_t left_ = -1;
    int16_t right_ = -1;
} impl;

}


[[tbd::responder(event="sound_processor_changed")]]
void reset_preset(const uint_par& channels, const uint_par&) {
    if (const auto _channels = channels_from_int(channels); _channels != CM_INVALID) {
        impl.reset_presets(_channels);
    }
}


Maybe<PresetID> Presets::get_active_preset(Channels channels) {
    return impl.get_active_preset(channels);
}

Error Presets::set_active_preset(const Channels channels, const PresetID preset_id) {
    return impl.set_active_preset(channels, preset_id);
}

Maybe<size_t> Presets::get_num_presets_for_channel(const Channels channels) {
    return impl.get_num_presets_for_channel(channels);
}

Maybe<size_t> Presets::get_num_presets_for_plugin(const PluginID plugin_id) {
    return impl.get_num_presets_for_plugin(plugin_id);
}

}