#pragma once

#include <Arduino.h>

// default log level is INFO
#ifndef TBD_LOG_LEVEL
#define TBD_LOG_LEVEL TBD_LOG_INFO
#endif


typedef enum {
    TBD_LOG_SILENT   = 0,
    TBD_LOG_ERROR    = 1,
    TBD_LOG_WARNING  = 2,
    TBD_LOG_INFO     = 3,
    TBD_LOG_DEBUG    = 4,
    TBD_LOG_VERBOSE  = 5,
    TBD_LOG_ALL      = 6,
} tbd_log_levels_t;


// if no baud rate is specified for logging, logging is disabled
#ifndef TBD_CLIENT_LOG_BAUD_RATE
    #define _TBD_LOG(level, tag, format, ...) do {} while (0)
#else
    #define _TBD_LOG(level, tag, format, ...) do { \
        if (level <= TBD_LOG_LEVEL) { \
            Serial.printf("%s:%i [%s] " format "\r\n", __FILE__, __LINE__, tag __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while(0)
#endif


#define TBD_LOGE( tag, format, ... ) _TBD_LOG(TBD_LOG_ERROR, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGW( tag, format, ... ) _TBD_LOG(TBD_LOG_WARNING, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGI( tag, format, ... ) _TBD_LOG(TBD_LOG_INFO, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGD( tag, format, ... ) _TBD_LOG(TBD_LOG_DEBUG, tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGV( tag, format, ... ) _TBD_LOG(TBD_LOG_VERBOSE, tag, format __VA_OPT__(,) __VA_ARGS__)
