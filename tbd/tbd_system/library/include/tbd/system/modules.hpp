#pragma once

#include <atomic>
#include <string>
#include <tbd/logging.hpp>
#include <tbd/system/tasks.hpp>


namespace tbd::system {
    
template<class ModuleT>
concept ModuleType = requires(ModuleT mod) {
    { mod.do_begin() } -> std::same_as<uint32_t>;
    { mod.do_work() } -> std::same_as<uint32_t>;
    { mod.do_cleanup() } -> std::same_as<uint32_t>;
};

enum class TaskState {
    created      = 0,
    initializing = 1,
    running      = 2,
    paused       = 3,
    terminating  = 4,
    done         = 5,
    crashed      = 6,
};

template<ModuleType ModuleT>
struct ModuleTask : ModuleT {
    ModuleTask(const char* name): _task(name) {}

    void begin() {
        if (_current_state != TaskState::created) {
            TBD_LOGE("module_task", "trying to start non idle task");
            return;
        }
        _desired_state = TaskState::running;
        _task.begin(&task_main_wrapper, this);
    
    }
    void end() {
        if (_current_state > TaskState::paused) {
            TBD_LOGE("module_task", "trying to shut down dead task");
            return;
        }
        _desired_state = TaskState::done;
    }

    void pause_processing() {
        if (_current_state > TaskState::paused) {
            TBD_LOGE("module_task", "trying to pause a dead task");
            return;
        }
        _desired_state = TaskState::paused;
    }
    void resume_processing() {
        if (_current_state != TaskState::paused) {
            TBD_LOGW("module_task", "attempting to resume ");
        }
        _desired_state = TaskState::running;
    }


protected:
    // owned by owner of task
    std::atomic<TaskState> _desired_state = TaskState::created;

    // for use in child process
    static void task_main_wrapper(void* param) {
        auto& instance = *reinterpret_cast<ModuleTask<ModuleT>*>(param);
        instance.task_main();
        instance._task.destroy();
    }

    void task_main() {
        uint32_t err;
        _current_state = TaskState::initializing;
        if ((err = ModuleT::do_begin()) != 0) {
            _current_state = TaskState::crashed;
            return;
        }
        _current_state = TaskState::running;
        while (_current_state < TaskState::terminating) {
            if (_current_state != TaskState::paused) {
                if ((err = ModuleT::do_work()) != 0) {
                    _current_state = TaskState::crashed;
                    return;
                }
            } else {
                // FIXME: some kind of slight delay
            }
        }

        _current_state = TaskState::initializing;
        if ((err = ModuleT::do_cleanup()) != 0) {
            _current_state = TaskState::crashed;
            return;
        }
        _current_state = TaskState::done;   
    }
    
    std::atomic<TaskState> _current_state = TaskState::created;
    tbd::system::Task _task;
};

}