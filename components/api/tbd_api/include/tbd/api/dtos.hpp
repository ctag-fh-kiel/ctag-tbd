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

struct SimpleCls {
    int_par simple1;
    trigger_par simple2;
};

struct ComplexCls {
    int_par complex1;
    SimpleCls complex2;
    trigger_par complex3;
};

struct VeryComplexCls {
    float_par vcomplex1;
    ComplexCls vcomplex2;
    SimpleCls vcomplex3;
    uint_par vcomplex4;
    ComplexCls vcomplex5;
    uint_par vcomplex6;
};

}
