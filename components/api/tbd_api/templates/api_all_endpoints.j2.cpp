{% import 'handlers.j2' as hs with context %}
#include <tbd/api/api_all_endpoints.hpp>

#include <tbd/api.hpp>

#include <tbd/logging.hpp>

#include <pb_encode.h>
#include <pb_decode.h>

#include <wrappers.pb.h>
{% for header in registry.proto_headers %}
#include <{{header}}>{%- endfor %}

namespace tbd::api { 
{%- for endpoint in registry.endpoints %}
{{hs.predeclare_endpoint(endpoint)}}
{%- endfor %}

{% for request in registry.request_types %}
// message {{request.name}}
template<> 
bool decode_message({{ request.name }}& message, const uint8_t* in_buffer, size_t len) {
    pb_istream_t stream = pb_istream_from_buffer(in_buffer, len);
    if (!pb_decode(&stream, {{request.name}}_fields, &message)) {
        TBD_LOGE("api", "failed to deserialize {{request.message_name}}: %s", PB_GET_ERROR(&stream));
        return false;
    }
    return true;
}
{% endfor %}

{% for response in registry.response_types %}
template<>
bool encode_message(const {{ response.name }}& message, uint8_t* buffer, size_t& buffer_size) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    if (!pb_encode(&stream, {{response.name}}_fields, &message)) {
        TBD_LOGE("api", "failed to serialize {{response.message_name}}: %s", PB_GET_ERROR(&stream));
        return false;
    }
    buffer_size = stream.bytes_written;
    return true;
}
{% endfor %}

{% for endpoint in registry.endpoints %}
{{ hs.handler_wrapper_impl(endpoint) }}
{% endfor %}

const size_t NUM_ENDPOINTS = {{ registry.endpoints | length }};

const Endpoint ENDPOINT_LIST[] = { {%- for endpoint in registry.endpoints %}
    { "{{ endpoint.name }}", {{ endpoint | request_id }}, {{ endpoint | response_id }}, &{{ endpoint | callback }} },
{%- endfor %} 
};

const MessageInfo REQUEST_LIST[] = { {%- for request in registry.request_types %}
    { "{{ request.name }}", MESSAGE_TYPE_REQUEST, {{ request.name }}_size },
    {%- endfor %}
};
const MessageInfo RESPONSE_LIST[] = { {%- for response in registry.response_types %}
    { "{{ response.name }}", MESSAGE_TYPE_REQUEST, {{ response.name }}_size },
    {%- endfor %} 
};

}

