#pragma once

#include <tbd/logging.hpp>
#include <tbd/sound_registry/module.hpp>
#include <tbd/errors.hpp>

#include <cinttypes>

TBD_NEW_ERR(INVALID_INPUT_ID, "invalid input id");
TBD_NEW_ERR(INVALID_SOUND_ID, "invalid sound id");
TBD_NEW_ERR(INVALID_PARAM_ID, "invalid parameter id");
TBD_NEW_ERR(INVALID_SOUND_PARAM_ID, "invalid sound parameter id");


namespace tbd::sound_registry::parameters {

using PluginID          = int32_t;
using ParameterID       = int32_t;
using ParameterCount    = uint32_t;
using Mapping           = int32_t;
using InputID           = uint32_t;

using PluginParameterID = uint32_t;

enum ParamType {
    INT_PARAM     = 0,
    UINT_PARAM    = 1,
    TRIGGER_PARAM = 2,
    FLOAT_PARAM   = 3,
    UFLOAT_PARAM  = 4,
};

struct PluginInfo {
    const char* name;
    ParameterID parameter_offset;
    ParameterCount num_parameters;
    ParameterCount num_ints;
    ParameterCount num_uints;
    ParameterCount num_triggers;
    ParameterCount num_floats;
    ParameterCount num_ufloats;
};

struct ParamInfo {
    const char* name;
    PluginID plugin_id;
    ParamType field_type;
};

extern const uint32_t NUM_PLUGINS;
extern const PluginInfo PLUGIN_LIST[];   
extern const uint32_t NUM_PARAMETERS;
extern const ParamInfo PARAMETER_LIST[];

inline uint32_t get_num_plugins() {
    return NUM_PLUGINS;
}

inline const PluginInfo* get_plugin_info(const PluginID plugin_id) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i out of range", plugin_id);
        return nullptr;
    }
    return &PLUGIN_LIST[plugin_id];
}

inline uint32_t get_num_parameters() {
    return NUM_PARAMETERS;
}

inline const ParamInfo* get_param_info(const ParameterID parameter_id) {
    if (parameter_id >= NUM_PARAMETERS) {
        TBD_LOGE(tag, "parameter ID %i out of range", parameter_id);
        return nullptr;
    }
    return &PARAMETER_LIST[parameter_id];
}

inline Error verify_parameter(const PluginID plugin_id, const ParameterID parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_parameters) {
        TBD_LOGE(tag, "parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline Error verify_mapping(const PluginID plugin_id, const ParameterID parameter_number, InputID input_id) {
    if (input_id >= N_CVS) {
        TBD_LOGE(tag, "input number %i is larger than number of available inputs", input_id);
        return TBD_ERR(INVALID_INPUT_ID);
    }
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_parameters) {
        TBD_LOGE(tag, "parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline Error verify_int_parameter(const PluginID plugin_id, const uint32_t parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_ints) {
        TBD_LOGE(tag, "int parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline Error verify_uint_parameter(const PluginID plugin_id, const uint32_t parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_uints) {
        TBD_LOGE(tag, "uint parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline Error verify_trigger_parameter(const PluginID plugin_id, const uint32_t parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_triggers) {
        TBD_LOGE(tag, "trigger parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline Error verify_float_parameter(const PluginID plugin_id, const uint32_t parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return TBD_ERR(INVALID_SOUND_ID);
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_floats + plugin_info.num_ufloats) {
        TBD_LOGE(tag, "float parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return TBD_ERR(INVALID_SOUND_PARAM_ID);
    }
    return TBD_OK;
}

inline const ParamInfo* get_plugin_parameter_info(const PluginID plugin_id, const uint32_t parameter_number) {
    if (plugin_id >= NUM_PLUGINS) {
        TBD_LOGE(tag, "plugin ID %i is out of range", plugin_id);
        return nullptr;
    }
    auto& plugin_info = PLUGIN_LIST[plugin_id];
    if (parameter_number >= plugin_info.num_parameters) {
        TBD_LOGE(tag, "parameter number %i for plugin %s is out of range", parameter_number, plugin_info.name);
        return nullptr;
    }
    return &PARAMETER_LIST[plugin_info.parameter_offset + parameter_number];
}

}
