#pragma once

#include <freertos/semphr.h>

namespace tbd::system {

struct Lock {

Lock() {
    // prepare threads and mutex
    _handle = xSemaphoreCreateMutex();
    if (processMutex == nullptr) {
        TBD_LOGE("SPM", "Fatal couldn't create mutex!");
    }
}

int take(uint32_t wait_time_ms = 0) {
    if (wait_time_ms == portMAX_DELAY) {
        return xSemaphoreTake(_handle, port) == pdTRUE;
    } else {
        return xSemaphoreTake(_handle, portTICK_PERIOD_MS(wait_time_ms)) == pdTRUE;
    }
   
}

void give() {
    xSemaphoreGive(_handle);
}

private:
    SemaphoreHandle_t _handle;
};

}
