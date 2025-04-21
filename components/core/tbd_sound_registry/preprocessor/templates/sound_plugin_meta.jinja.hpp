#pragma once

#include <string>
#include <ctag/reflect/properties.hpp>

{% for header in headers %}
#include <tbd/sounds/{{ header }}>
{%- endfor %}

namespace CTAG {
namespace SP {

{% for cls in sound_processors %}
struct {{ cls.meta_name }} {
    using Accessor = void*(*)({{ cls.cls_name }}&);

    struct PropertyMeta {
        reflect::PropertyType type;
        Accessor accessor;
    };

    {{ cls.meta_name }}({{ cls.cls_name }}& instance) : instance_(instance) {}

    template<typename T>
    T set(const std::string& property_name, const T& value) {
        return access_property<T>(property_name) = value;
    } 

    template<typename T>
    T get(const std::string& property_name) const {
        return access_property_readonly<T>(property_name);
    }

private:
    {%- for property in cls.properties %}
    static void* access_{{property.cls_name}}({{ cls.cls_name }}& instance) { return &instance.{{property.cls_name}}; }
    {%- endfor %}

    template<typename T>
    const T& access_property_readonly(const std::string& property_name) const {
        if (!properties_.contains(property_name)) {
            // do something error like here
            return T();
        }
        auto cls_meta = properties_[property_name];
        if (cls_meta.type != reflect::get_property_type<T>()) {
            // do somethings error like here
            return T();
        }
        return cls_meta.accessor();
    }

    template<typename T>
    T& access_property(const std::string& property_name) {
        return const_cast<T&>(access_property_readonly<T>(property_name));
    }

    {{ cls.cls_name }}& instance_;
    static std::map<std::string, PropertyMeta> properties_;
};

{% endfor %}

}
}
