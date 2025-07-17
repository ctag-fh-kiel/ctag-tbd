#pragma once

#include <tbd/api/packet.hpp>
#include <tbd/errors.hpp>

namespace tbd::api {

constexpr int32_t NO_MESSAGE = -1;

struct MessageInfo {
    const char* name;
    int32_t size;
};

}
