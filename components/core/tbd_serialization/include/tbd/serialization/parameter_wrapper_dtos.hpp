#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd {

struct [[tbd::dto]] IntParWrapper {
    int_par value;
};


struct [[tbd::dto]] UintParWrapper {
    uint_par value;
};


struct [[tbd::dto]] FloatParWrapper {
    float_par value;
};


struct [[tbd::dto]] UfloatParWrapper {
    ufloat_par value;
};


struct [[tbd::dto]] TriggerParWrapper {
    trigger_par value;
};


struct [[tbd::dto]] StrParWrapper {
    str_par value;
};

}
