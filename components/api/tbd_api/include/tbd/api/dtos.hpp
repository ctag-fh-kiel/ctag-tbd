#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd::api {

struct ApiVersion {
    uint_par version;
    uint_par core_hash;
    uint_par base_hash;
    uint_par api_hash;
};

struct DeviceInfo {
    int_par api_version;
    int_par api_hash;
};

struct Foo {
    int_par foo1;
    trigger_par foo2;
};

struct Bar {
    float_par bar1;
    Foo bar2;
    uint_par bar3;
};

}
