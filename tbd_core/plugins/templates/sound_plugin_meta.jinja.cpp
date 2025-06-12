#include "{{ meta_header }}"

namespace tbd::sounds::meta {

{{ meta_name }}::IntSetter {{ meta_name }}::int_setters[{{ plugin.num_ints }}] = { {%- for param in plugin.int_params %}
    {{ meta_name }}::set_{{ param.snake_name }},
{%- endfor %}
};

{{ meta_name }}::UIntSetter {{ meta_name }}::uint_setters[{{ plugin.num_uints }}] = { {%- for param in plugin.uint_params %}
    {{ meta_name }}::set_{{ param.snake_name }},
{%- endfor %}
};

{{ meta_name }}::TriggerSetter {{ meta_name }}::trigger_setters[{{ plugin.num_triggers }}] = { {%- for param in plugin.trigger_params %}
    {{ meta_name }}::set_{{ param.snake_name }},
{%- endfor %}
};

{{ meta_name }}::FloatSetter {{ meta_name }}::float_setters[{{ plugin.num_floats + plugin.num_ufloats }}] = { 
{%- for param in plugin.float_params %}
    {{ meta_name }}::set_{{ param.snake_name }},
{%- endfor %}
{%- for param in plugin.ufloat_params %}
    {{ meta_name }}::set_{{ param.snake_name }},
{%- endfor %}
};
}
