#pragma once

#include <tbd/system/port/locks.hpp>

namespace tbd::system {

template<class LockT>
concept LockType = requires(LockT lock, uint32_t _uint32_t) {
    { lock.take(_uint32_t) } -> std::same_as<void>;
    { lock.give() } -> std::same_as<void>;
};

// check if Lock provided py port satisfies requirements
static_assert(LockType<Lock>);

}