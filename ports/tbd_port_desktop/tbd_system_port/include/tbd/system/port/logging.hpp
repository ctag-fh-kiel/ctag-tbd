#pragma once

#include <tbd/system/common/logging_pre.hpp>

#include <stdio.h>

TBD_C_BEGIN


#define _TBD_LOG(level, tag, format, ...) do {                      \
    if (level <= TBD_LOG_LEVEL) {                                    \
        printf("[%s]" format, tag __VA_OPT__(,) __VA_ARGS__);        \
    }                                                                \
} while(0)


#define TBD_LOGE( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGW( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGI( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGD( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGV( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)

TBD_C_END
