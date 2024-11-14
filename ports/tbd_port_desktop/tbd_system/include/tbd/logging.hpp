#pragma once

#include <tbd/private/logging_levels.hpp>

#define _TBD_WRITE_TO_LOG(tag, format, ...) printf(format "\n", ##__VA_ARGS__)
#define _TBD_LOG_IF_LEVEL_ENABLED(level, tag, format, ...) do { \
    if (level <= TBD_LOG_LEVEL) {                               \
        _TBD_WRITE_TO_LOG(level, tag, format)                   \
    }                                                           \
} while(0)

#define TBD_LOGE( tag, format, ... ) _TBD_LOG_IF_LEVEL_ENABLED(TBD_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define TBD_LOGW( tag, format, ... ) _TBD_LOG_IF_LEVEL_ENABLED(TBD_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define TBD_LOGI( tag, format, ... ) _TBD_LOG_IF_LEVEL_ENABLED(TBD_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define TBD_LOGD( tag, format, ... ) _TBD_LOG_IF_LEVEL_ENABLED(TBD_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define TBD_LOGV( tag, format, ... ) _TBD_LOG_IF_LEVEL_ENABLED(TBD_LOG_ERROR, tag, format, ##__VA_ARGS__)
