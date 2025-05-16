#include <tbd/api.hpp>

#include <tbd/reflection/parameter_types.h>
#include <tbd/api/api_all_endpoints.hpp>

#include <api_types.pb.h>
#include <cinttypes>

namespace tbd::api {

[[tbd::endpoint]]
uint32_t get_num_endpoints(uint_par& num_endpoints) {
    num_endpoints = NUM_ENDPOINTS;
    return errors::SUCCESS;
}

[[tbd::endpoint]] 
uint32_t get_endpoint_name(const uint_par& endpoint, str_par& name) {
    name = ENDPOINT_LIST[endpoint].path;
    return 0;
}

[[tbd::endpoint]] 
uint32_t get_device_info(DeviceInfo& device_info) {
    device_info.api_version = 1;
    device_info.api_hash = 42;
    return errors::SUCCESS;
}

}
