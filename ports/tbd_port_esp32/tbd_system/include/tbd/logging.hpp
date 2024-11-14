#include <esp_log.h>

#include <tbd/private/logging_levels.hpp>

#if (ESP_LOG_NONE != TBD_LOG_SILENT \
    || ESP_LOG_ERROR != TBD_LOG_ERROR \
    || ESP_LOG_WARN != TBD_LOG_WARN \
    || ESP_LOG_INFO != TBD_LOG_INFO \
    || ESP_LOG_DEBUG != TBD_LOG_DEBUG \
    || ESP_LOG_VERBOSE != TBD_LOG_VERBOSE \
    || ESP_LOG_MAX != TBD_LOG_ALL)
#error "idf logging interface diverges from TBD logging"
#endif

#define TBD_LOGE( tag, format, ... ) ESP_LOGE(tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGW( tag, format, ... ) ESP_LOGW(tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGI( tag, format, ... ) ESP_LOGI(tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGD( tag, format, ... ) ESP_LOGD(tag, format __VA_OPT__(,) __VA_ARGS__)
#define TBD_LOGV( tag, format, ... ) ESP_LOGV(tag, format __VA_OPT__(,) __VA_ARGS__)
