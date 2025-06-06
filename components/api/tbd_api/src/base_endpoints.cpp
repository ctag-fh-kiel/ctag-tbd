#include <tbd/parameter_types.hpp>
#include <tbd/api/api_all_endpoints.hpp>
#include <tbd/api/api_all_events.hpp>

#include <api_types.pb.h>
#include <cinttypes>

#include <tbd/errors.hpp>


namespace tbd::api {

[[tbd::endpoint]]
Error get_api_version(ApiVersion& version) {
    version.version = API_VERSION;
    version.base_hash = BASE_API_HASH;
    version.api_hash = API_HASH;
    return TBD_OK;
}

[[tbd::endpoint]]
Error update_device() {
    return TBD_OK;
}

[[tbd::endpoint]]
Error reset_device() {
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_num_endpoints(uint_par& num_endpoints) {
    num_endpoints = NUM_ENDPOINTS;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_endpoint_name(const uint_par& endpoint_id, str_par& name) {
    if (endpoint_id >= NUM_ENDPOINTS) {
        return TBD_ERR(API_BAD_ENDPOINT);
    }
    name = ENDPOINT_LIST[endpoint_id].path;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_device_info(DeviceInfo& device_info) {
    device_info.api_version = 1;
    device_info.api_hash = API_HASH;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_num_errors(uint_par& num_errors) {
    num_errors = errors::get_num_errors();
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_error_name(const uint_par& err_id, str_par& name) {
    if (err_id >= errors::get_num_errors()) {
        return TBD_ERR(API_BAD_ERROR);
    }
    name = errors::get_error_name(err_id);
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_error_message(const uint_par& err_id, str_par& msg) {
    if (err_id >= errors::get_num_errors()) {
        return TBD_ERR(API_BAD_ERROR);
    }
    msg = errors::get_error_message(err_id);
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_num_events(uint_par& num_endpoints) {
    num_endpoints = NUM_EVENTS;
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_event_name(const uint_par& event_id, str_par& name) {
    if (event_id >= NUM_EVENTS) {
        return TBD_ERR(API_BAD_EVENT);
    }
    name = EVENT_LIST[event_id].path;
    return TBD_OK;
}

}
