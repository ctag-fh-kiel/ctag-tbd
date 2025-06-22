#include <tbd/parameter_types.hpp>
#include <tbd/sound_registry/factory.hpp>
#include <api_types.pb.h>


namespace sp_pars = tbd::sound_registry::parameters;

namespace tbd::sound_registry {

using namespace sound_processor::channels;


[[tbd::endpoint]]
Error get_num_sounds(uint_par& num_sounds) {
    num_sounds = sp_pars::get_num_plugins();
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_name(const uint_par& sound_id, str_par& sound_name) {
    const auto plugin_info = sp_pars::get_plugin_info(sound_id);
    if (plugin_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_ID);
    }
    sound_name = plugin_info->name;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_num_params(const uint_par& sound_id, int_par& num_params) {
    const auto plugin_info = sp_pars::get_plugin_info(sound_id);
    if (plugin_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_ID);
    }
    num_params = plugin_info->num_parameters;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_param_name(const uint_par& sound_id, const uint_par& param_id, str_par& param_name) {
    const auto param_info = sp_pars::get_plugin_parameter_info(sound_id, param_id);
    if (param_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    param_name = param_info->name;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_param_info(const uint_par& sound_id, const uint_par& param_id, str_par& param_name) {
    const auto param_info = sp_pars::get_plugin_parameter_info(sound_id, param_id);
    if (param_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    param_name = param_info->name;
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_sound_plugin(const uint_par& channel, const uint_par& sound_id) {
    const auto _channel = channels_from_int(channel);
    if (_channel == CM_INVALID) {
        return TBD_ERR(SOUND_REGISTRY_BAD_CHANNEL_MAPPING);
    }
    return factory::set_plugin(_channel, sound_id);
}

[[tbd::endpoint]]
Error get_active_plugins(ActivePlugins& active_plugins) {
    active_plugins.is_stereo = ActiveSoundProcessors::is_stereo();
    active_plugins.left = ActiveSoundProcessors::on_left();
    active_plugins.right = ActiveSoundProcessors::on_right();
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_plugin_int_param(const uint_par& channel, const uint_par& param_id, const int_par& value) {
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_plugin_uint_param(const uint_par& channel, const uint_par& param_id, const uint_par& value) {
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_plugin_float_param(const uint_par& channel, const uint_par& param_id, const float_par& value) {
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_plugin_ufloat_param(const uint_par& channel, const uint_par& param_id, const ufloat_par& value) {
    return TBD_OK;
}

[[tbd::endpoint]]
Error set_plugin_trigger_param(const uint_par& channel, const uint_par& param_id, const trigger_par& value) {
    return TBD_OK;
}

}
