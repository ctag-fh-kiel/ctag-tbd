#pragma once

#include <tbd/system/port/logging.hpp>

#ifndef TBD_LOGE
    #error "port needs to supply TBD_LOGE macro function"
#endif

#ifndef TBD_LOGW
    #error "port needs to supply TBD_LOGW macro function"
#endif

#ifndef TBD_LOGI
    #error "port needs to supply TBD_LOGI macro function"
#endif

#ifndef TBD_LOGD
    #error "port needs to supply TBD_LOGD macro function"
#endif

#ifndef TBD_LOGV
    #error "port needs to supply TBD_LOGV macro function"
#endif
