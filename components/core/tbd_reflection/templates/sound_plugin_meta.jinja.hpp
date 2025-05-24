{% import 'accessors.j2' as ac with context %}
#pragma once

#include <cmath>

#include <tbd/sound_processor.hpp>
#include <tbd/sound_processor/dynamic_sound_processor.hpp>
#include <tbd/sound_processor/parameters.hpp>

#include <tbd/sounds/{{ plugin_header }}>

namespace tbd::sounds::meta {

class {{ meta_name }} : public {{ plugin.full_name }}, public sound_processor::DynamicSoundProcessor {
    {{ meta_name }}() : sound_processor::DynamicSoundProcessor({{ plugin_id }}) {}

    virtual void Process(const sound_processor::ProcessData& data) {
        process_updates();
        populate_parameters(data);
        {{ plugin.name }}::Process(data);
    }

    void populate_parameters(const sound_processor::ProcessData& data) {
        {%- for index, param in params %}
         // mapping parameter {{index}}: {{param.name}}
        if (mappings_[{{index}}] >= 0) {
            auto value = {{ac.value_from_cv(index, param)}};
            {{param|setter}}(*this, data.cv[mappings_[{{index}}]]);
        }
        {%- endfor %}
    }

    void process_updates() {
        if (operation_cleared_) {
            return;
        }
        switch (operation_.type) {
            case WriteOperation::SET_MAPPING: {
                mappings_[operation_.parameter] = operation_.value.mapping;
                break;
            }
            case WriteOperation::SET_INT: {
                auto setter_id = operation_.parameter;
                int_setters[setter_id](*this, operation_.value.int_param);
                break;
            }
            case WriteOperation::SET_UINT: {
                auto setter_id = operation_.parameter - {{ plugin.num_ints }};
                uint_setters[setter_id](*this, operation_.value.uint_param);
                break;
            }
            case WriteOperation::SET_TRIGGER: {
                auto setter_id = operation_.parameter - {{ plugin.num_ints }} - {{ plugin.num_uints }};
                trigger_setters[setter_id](*this, operation_.value.trigger_param);
                break;
            }
            case WriteOperation::SET_FLOAT: {
                auto setter_id = operation_.parameter - {{ plugin.num_ints }} - {{ plugin.num_uints }} - {{ plugin.num_triggers }}; 
                int_setters[setter_id](*this, operation_.value.float_param);
                break;
            }
            default:
                break;
        }
        operation_cleared_ = true;
    }

    {% if plugin.num_ints %}
     // {{plugin.num_ints}} int params
    {%- for param in plugin.int_params %}
    {{ac.int_setter_impl(plugin, param)}}
    {%- endfor %}
    {% else %}
     // no int params 
    {% endif %}
    
    {% if plugin.num_uints %}
     // {{plugin.num_uints}} uint params
    {% for param in plugin.uint_params %}
    {{ac.uint_setter_impl(plugin, param)}}
    {%- endfor %}
    {% else %}
     // no uint params 
    {% endif %}


    {% if plugin.num_triggers %}
     // {{plugin.num_triggers}} trigger params
    {% for param in plugin.trigger_params %}
    {{ac.trigger_setter_impl(plugin, param)}}
    {%- endfor %}
    {% else %}
     // no trigger params 
    {% endif %}
    
    {% if plugin.num_floats or plugin.num_ufloats %}
     // {{plugin.num_floats + plugin.num_ufloats }} float params
    {% for param in plugin.float_params %}
    {{ac.float_setter_impl(plugin, param)}}
    {%- endfor %}
    {%- for param in plugin.ufloat_params %}
    {{ac.float_setter_impl(plugin, param)}}
    {%- endfor %}
    {% else %}
     // no float params 
    {% endif %} 

private:
    sound_processor::parameters::Mapping mappings_[{{ plugin.num_params }}];
    
    using IntSetter = void(*)({{ meta_name }}&, int_par);
    static IntSetter int_setters[{{ plugin.num_ints }}];

    using UIntSetter = void(*)({{ meta_name }}&, uint_par);
    static UIntSetter uint_setters[{{ plugin.num_uints }}];
    
    using TriggerSetter = void(*)({{ meta_name }}&, trigger_par);
    static TriggerSetter trigger_setters[{{ plugin.num_triggers }}];
    
    using FloatSetter = void(*)({{ meta_name }}&, float);
    static FloatSetter float_setters[{{ plugin.num_floats + plugin.num_ufloats}}];
};

}

