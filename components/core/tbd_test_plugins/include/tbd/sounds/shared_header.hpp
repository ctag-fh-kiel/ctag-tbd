#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd::test_sounds {

struct SharedGroupType1 {
    float_par sg1_p1;
    float_par sg1_p2;
    trigger_par sg1_p3;
    int_par sg1_p4;
};

struct SharedGroupType2 {
    trigger_par sg2_p1;
    trigger_par sg2_p2;
    float_par sg2_p3;
    trigger_par sg2_p4;
    uint_par sg2_p5;
};

}