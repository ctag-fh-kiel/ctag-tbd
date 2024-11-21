#pragma once

#include <thread>
#include <chrono>

#include <tbd/system/cpu_cores.hpp>



namespace tbd::system {

struct Task {
    using task_func_type = void(*)(void*);

    Task(const char* name) : _name(name) {}

    bool is_running() {
        return _thread.joinable();
    }

    void begin(
        task_func_type task_main, 
        void* context, 
        CpuCore core_id = CpuCore::system,
        uint8_t priority = 0) 
    {
        _thread = std::thread(task_main, context);
    }

    static void sleep(uint32_t time_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
    }

private:
    const char* _name;
    std::thread _thread;
};

}
