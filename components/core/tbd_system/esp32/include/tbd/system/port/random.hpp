#pragma once

#include <tbd/system/common/random_pre.hpp>
#include "xtensa/core-macros.h"


namespace tbd::system {

inline uint32_t get_seed() { return XTHAL_GET_CCOUNT(); }

}
