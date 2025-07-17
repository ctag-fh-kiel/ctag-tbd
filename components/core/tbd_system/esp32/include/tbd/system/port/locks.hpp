#pragma once

#include <tbd/esp32_only.hpp>

#include <tbd/logging.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace tbd::system {

struct LockImpl {

    LockImpl() {
        // prepare threads and mutex
        _handle = xSemaphoreCreateMutex();
        if (!is_valid()) {
            TBD_LOGE("tbd_system", "lock creation failed");
        }
    }

    ~LockImpl() {
        if (!is_valid()) {
            TBD_LOGE("tbd_system", "attempting to delete invalid lock");
        }
        vSemaphoreDelete(_handle);
    }

    bool is_valid() const {
        return _handle != nullptr;
    }

    bool lock(uint32_t wait_time_ms = 0) {
        if (!is_valid()) {
            TBD_LOGE("tbd_system", "attempting to lock invalid lock");
            return false;
        }

        auto wait_time = wait_time_ms == 0 ? portMAX_DELAY : wait_time_ms;
        if (xSemaphoreTake(_handle, wait_time) != pdTRUE) {
            TBD_LOGE("tbd_system", "failed to acquire lock");
            return false;
        }

        return true;
    }

    bool try_lock() {
        if (!is_valid()) {
            TBD_LOGE("tbd_system", "attempting to lock invalid lock");
            return false;
        }

        return xSemaphoreTake(_handle, 0) == pdTRUE;
    }


    void unlock() {
            if (!is_valid()) {
            TBD_LOGE("tbd_system", "attempting to unlock invalid lock");
            return;
        }

        xSemaphoreGive(_handle);
    }

private:
    SemaphoreHandle_t _handle;
};

}
