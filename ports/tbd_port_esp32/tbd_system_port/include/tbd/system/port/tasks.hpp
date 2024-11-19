#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>

#include <tbd/system/cpu_cores.hpp>


namespace tbd::system {


struct Task {
    using task_func_type = void(*)(void*);

    Task(const char* name) : _name(name), _handle(nullptr) {}

    ~Task() {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    void begin(
        task_func_type task_main, 
        void* context, 
        CpuCore core_id = CpuCore::system,
        uint8_t priority = 0) 
    {
        xTaskCreatePinnedToCore(
            task_main, 
            _name, 
            4096, 
            context, 
            tskIDLE_PRIORITY + priority,
            &_handle, 
            0);
    }

    static void sleep(uint32_t time_ms) {
        vTaskDelay(time_ms / portTICK_PERIOD_MS);
    }

    void destroy() {
         vTaskDelete(_handle);
    }

private:
    const char* _name;
    TaskHandle_t _handle;
};

}
