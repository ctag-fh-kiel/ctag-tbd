#include <tbd/parameter_types.hpp>
#include <tbd/sound_processor/parameters.hpp>
#include <api_types.pb.h>

#include "errors.hpp"


namespace sp_pars = tbd::sound_processor::parameters;

namespace tbd::sounds {

[[tbd::endpoint]]
Error get_num_sounds(uint_par& num_sounds) {
    num_sounds = sp_pars::get_num_plugins();
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_name(const uint_par& sound_id, str_par& sound_name) {
    auto plugin_info = sp_pars::get_plugin_info(sound_id);
    if (plugin_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_ID);
    }
    sound_name = plugin_info->name;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_num_params(const uint_par& sound_id, int_par& num_params) {
    auto plugin_info = sp_pars::get_plugin_info(sound_id);
    if (plugin_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_ID);
    }
    num_params = plugin_info->num_parameters;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_sound_params_name(const uint_par& sound_id, const uint_par& param_id, str_par& param_name) {
    auto param_info = sp_pars::get_plugin_parameter_info(sound_id, param_id);
    if (param_info == nullptr) {
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    param_name = param_info->name;
    return TBD_OK;
}

}
