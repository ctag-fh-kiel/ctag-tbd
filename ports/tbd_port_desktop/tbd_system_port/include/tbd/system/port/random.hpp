#pragma once

#include <tbd/system/common/random_pre.hpp>
#include <random>


namespace tbd::system {

inline uint32_t get_seed() { 
    std::random_device rd;
    return rd();
}

}
