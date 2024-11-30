#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/idf_additions.h>

#include <tbd/system/cpu_cores.hpp>
#include <tbd/logging.hpp>


namespace tbd::system {

struct Task {
    using task_func_type = void(*)(void*);

    Task(const char* name) : _name(name), _handle(nullptr) {}

    ~Task() {
        if (is_running()) {
            vTaskDelete(_handle);
        }
    }

    bool is_running() {
        return _handle != nullptr;
    }

    void begin(
        task_func_type task_main, 
        void* context,
        CpuCore core_id = CpuCore::system,
        uint32_t stack_size = 4096,
        uint8_t priority = 0) 
    {
        _task_data.task_main = task_main;
        _task_data.contxt = context;

        auto err = xTaskCreatePinnedToCore(
            task_main, 
            _name, 
            stack_size,
            &_task_data, 
            tskIDLE_PRIORITY + priority,
            &_handle, 
            static_cast<BaseType_t>(core_id));
        if (err != pdPASS) {
            TBD_LOGE("tbd_system", "failed to create task %s", _name);
            _handle = nullptr;
        }
    }

    static void sleep(uint32_t time_ms) {
        vTaskDelay(time_ms / portTICK_PERIOD_MS);
    }

private:
    struct Context {
        task_func_type task_main;
        void* contxt;
    } _task_data;

    [[noreturn]] static void task_wrapper (void* task_arg) {
        auto task_data = reinterpret_cast<Context*>(task_arg);
        task_data->task_main(task_data->contxt);

        while (true) {
            sleep(10);
        }
    }

    const char* _name;
    TaskHandle_t _handle;
};

}
