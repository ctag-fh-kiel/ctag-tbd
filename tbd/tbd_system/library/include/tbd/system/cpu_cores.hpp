#pragma once

#include <tbd/system/common/cpu_cores.hpp>
#include <tbd/system/port/cpu_cores.hpp>


namespace tbd::system {

// check if Task provided py port satisfies requirements
static_assert(TimeoutGuardType<TimeoutGuard>, "port task type does not fullfil requirements");

}