#pragma once

#include <tbd/system/common/locks_pre.hpp>
#include <tbd/system/port/locks.hpp>

namespace tbd::system {

// check if Lock provided py port satisfies requirements
static_assert(LockType<LockImpl>);

using LockGuard = LockGuardTemplate<LockImpl>;
using Lock = LockDecorator<LockImpl>;

}