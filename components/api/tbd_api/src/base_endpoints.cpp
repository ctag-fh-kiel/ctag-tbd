#include <tbd/parameter_types.h>
#include <tbd/api/api_all_endpoints.hpp>

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
Error get_endpoint_name(const uint_par& endpoint, str_par& name) {
    name = ENDPOINT_LIST[endpoint].path;
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
Error get_error_name(const uint_par& err, str_par& name) {
    name = errors::get_error_name(err);
    return TBD_OK;
}

[[tbd::endpoint]]
Error get_error_message(const uint_par& err, str_par& msg) {
    msg = errors::get_error_message(err);
    return TBD_OK;
}

}
