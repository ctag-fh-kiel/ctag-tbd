#include <{{ header_name }}>

namespace CTAG {
namespace SP {

{% for cls in sound_processors %}

std::map<std::string, {{ cls.meta_name }}::PropertyMeta> {{ cls.meta_name }}::properties_ = {
    {%- for property in cls.properties %}
    {
        "{{ property.name }}", {
            reflect::PropertyType::{{ property.type }},
            {{ cls.meta_name }}::access_{{ property.name }}
        } 
    },
    {%- endfor %}
};
{% endfor %}

}
}
