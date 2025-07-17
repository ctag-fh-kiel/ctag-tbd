#pragma once

#include <tbd/parameter_types.hpp>

namespace tbd::sound_registry {

struct ActivePlugins {
    trigger_par is_stereo;
    int_par left;
    int_par right;
};

}