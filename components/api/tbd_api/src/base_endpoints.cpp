#include <tbd/api.hpp>

#include "api_base.pb.h"
#include <cinttypes>

namespace tbd::api {

[[tbd::endpoint]] 
uint32_t get_device_info(const Foo& request, DeviceInfo& device_info) {
    device_info.api_version = 1;
    device_info.api_hash = 42;
    return errors::SUCCESS;
}

[[tbd::endpoint]]
uint32_t get_num_endpoints() {
    
    return errors::SUCCESS;
}

[[tbd::endpoint]] 
uint32_t set_foo(const Foo& device_info) {
    return 0;
}

[[tbd::endpoint]] 
uint32_t get_bar(Bar& request) {
    return 0;
}

[[tbd::endpoint]] 
uint32_t trigger_blub() {
    return 0;
}

}