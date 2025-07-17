#pragma once

#include <tbd/system/common/cpu_cores.hpp>
#include <tbd/system/port/cpu_cores.hpp>



#ifndef TBD_TIMEOUT_ENSURE_OPS_PER_SECOND
    #error "port cpu_cores.h does not provide TBD_TIMEOUT_ENSURE_OPS_PER_SECOND function macro"
#endif

namespace tbd::system {

// check if Task provided py port satisfies requirements
static_assert(TimeoutGuardType<TimeoutGuard>, "port task type does not fullfil requirements");

}