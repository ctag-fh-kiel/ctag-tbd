#include <stdio.h>

#define ESP_LOGE( tag, format, ... ) printf(format, ##__VA_ARGS__)
#define ESP_LOGW( tag, format, ... ) printf(format, ##__VA_ARGS__)
#define ESP_LOGI( tag, format, ... ) printf(format, ##__VA_ARGS__)
#define ESP_LOGD( tag, format, ... ) //printf(format, ##__VA_ARGS__)
#define ESP_LOGV( tag, format, ... ) //printf(format, ##__VA_ARGS__)
