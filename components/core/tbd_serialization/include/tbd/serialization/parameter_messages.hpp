#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd {

struct IntParWrapper {
    int_par value;
};

struct UintParWrapper {
    uint_par value;
};

struct FloatParWrapper {
    float_par value;
};

struct UfloatParWrapper {
    ufloat_par value;
};

struct TriggerParWrapper {
    trigger_par value;
};

struct StrParWrapper {
    str_par value;
};

}
