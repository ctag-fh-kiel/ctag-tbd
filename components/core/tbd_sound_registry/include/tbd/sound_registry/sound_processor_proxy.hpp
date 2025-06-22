#pragma once

#include <tbd/errors.hpp>
#include <tbd/sound_registry/all_sound_processors.hpp>
#include <tbd/parameter_types.hpp>
#include <tbd/sound_registry/plugin_meta_base.hpp>


namespace tbd::sound_registry {



struct SoundProcessorParamProxy {
    explicit SoundProcessorParamProxy(PluginMetaBase* processor) : processor_(processor) {}

    Error set_int(parameters::ParameterID, int_par value) const;
    Error set_uint(parameters::ParameterID, uint_par value) const;
    Error set_float(parameters::ParameterID, float_par value) const;
    Error set_ufloat(parameters::ParameterID, ufloat_par value) const;
    Error set_trigger(parameters::ParameterID, trigger_par value) const;

protected:
    PluginMetaBase* processor_;
};

inline Error SoundProcessorParamProxy::set_int(const parameters::ParameterID param_id, const int_par value) const {
    if (processor_ == nullptr) {
        return TBD_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
    }
    processor_->set_param(param_id, value);
    return TBD_OK;
}

inline Error SoundProcessorParamProxy::set_uint(const parameters::ParameterID param_id, const uint_par value) const {
    if (processor_ == nullptr) {
        return TBD_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
    }
    processor_->set_param(param_id, value);
    return TBD_OK;
}

inline Error SoundProcessorParamProxy::set_float(const parameters::ParameterID param_id, const float_par value) const {
    if (processor_ == nullptr) {
        return TBD_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
    }
    processor_->set_param(param_id, value);
    return TBD_OK;
}

inline Error SoundProcessorParamProxy::set_ufloat(const parameters::ParameterID param_id, const ufloat_par value) const {
    if (processor_ == nullptr) {
        return TBD_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
    }
    processor_->set_param(param_id, value);
    return TBD_OK;
}

inline Error SoundProcessorParamProxy::set_trigger(const parameters::ParameterID param_id, const trigger_par value) const {
    if (processor_ == nullptr) {
        return TBD_ERR(SOUND_REGISTRY_NO_PLUGIN_PRESENT);
    }
    processor_->set_param(param_id, value);
    return TBD_OK;
}

}