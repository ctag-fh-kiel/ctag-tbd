#pragma once

#include <atomic>
#include <string>
#include <tbd/logging.hpp>
#include <tbd/system/tasks.hpp>
#include <tbd/system/cpu_cores.hpp>


namespace tbd::system {
    
template<class ModuleT>
concept TaskModuleImplType = requires(ModuleT mod) {
    { mod.do_begin() } -> std::same_as<uint32_t>;
    { mod.do_work() } -> std::same_as<uint32_t>;
    { mod.do_cleanup() } -> std::same_as<uint32_t>;
};

template<class ModuleT>
concept TaskModuleType = requires(ModuleT mod, bool _bool) {
    { mod.begin(_bool) } -> std::same_as<uint32_t>;
    { mod.begin() } -> std::same_as<uint32_t>;
    { mod.end(_bool) } -> std::same_as<uint32_t>;
    { mod.end() } -> std::same_as<uint32_t>;
    { mod.pause_processing(_bool) } -> std::same_as<uint32_t>;
    { mod.pause_processing() } -> std::same_as<uint32_t>;
};

enum class TaskState : uint8_t {
    created      = 0,
    initializing = 1,
    running      = 2,
    paused       = 3,
    terminating  = 4,
    done         = 5,
    crashed      = 6,
    ipc_error    = 7,
};


template<TaskModuleImplType ModuleT, CpuCore core_id, uint32_t stack_size = 4096>
struct TaskModule : ModuleT {
    TaskModule& operator=(const TaskModule&) = delete;

    TaskModule(const char* name) :
        _desired_state(TaskState::created), _task(name),
        _current_state(TaskState::created) {}

    TaskState state() const {
        return _current_state;
    }

    uint32_t begin(bool wait = false) {
        TBD_LOGV("module_task", "starting task %s", _task.name());

        if (_current_state != TaskState::created) {
            TBD_LOGE("module_task", "trying to start non idle task");
            return 1;
        }
        _desired_state.store(TaskState::running);
        _task.begin(&task_main_wrapper, this, core_id, stack_size);

        if (!wait) {
            return 0;
        }

        // wait until task main has started up
        while (_current_state == TaskState::created) {
            Task::sleep(10);
        }

        TBD_LOGV("module_task", "task %s initializing", _task.name());

        // wait until task has progressed beyond initialization
        while (_current_state < TaskState::running) {
            Task::sleep(10);
        }

        if (_current_state == TaskState::crashed) {
            TBD_LOGE("module_task", "task %s failed to start", _task.name());
        }
        TBD_LOGV("module_task", "task %s has started", _task.name());
        return 0;
    }

    uint32_t end(bool wait = false) {
        TBD_LOGV("module_task", "shutting down task %s", _task.name());

        if (_current_state > TaskState::paused) {
            TBD_LOGE("module_task", "trying to shut down dead task");
            return 1;
        }

        if (_current_state < TaskState::initializing) {
            TBD_LOGW("module_task", "trying to shut down a starting task");
            return 1;
        }

        _desired_state = TaskState::done;
        if (_current_state == TaskState::created) {
            TBD_LOGW("module_task", "shutting down a task that has not yet started");
            _current_state = TaskState::done;
            return 1;
        }

        if (!wait) {
            return 0;
        }

        // wait indefinitely
        while (_current_state < _desired_state) {
            Task::sleep(10);
        }
        TBD_LOGV("module_task", "task %s ended", _task.name());
        return 0;
    }

    uint32_t pause_processing(bool wait = false) {
        TBD_LOGV("module_task", "pausing task %s", _task.name());

        if (_current_state > TaskState::paused) {
            TBD_LOGE("module_task", "trying to pause a dead task");
            return 1;
        }

        if (_current_state < TaskState::running) {
            TBD_LOGW("module_task", "trying to pause a starting task");
            return 1;
        }

        _desired_state = TaskState::paused;

        if (!wait) {
            return 0;
        }

        // wait indefinitely
        while (_current_state < _desired_state) {
            Task::sleep(10);
        }
        TBD_LOGV("module_task", "task %s ended", _task.name());
        return 0;
    }

    uint32_t resume_processing(bool wait = false) {
        if (_current_state == TaskState::initializing) {
            TBD_LOGW("module_task", "attempting to resume a staring task");
            _desired_state = TaskState::running;
            return 1;
        }

        if (_current_state == TaskState::running) {
            TBD_LOGW("module_task", "attempting to resume a running");
            _desired_state = TaskState::running;
            return 1;
        }

        if (_current_state == TaskState::created) {
            TBD_LOGE("module_task", "task has not started, can not resume");
            return 1;
        }

        if (_current_state > TaskState::paused) {
            TBD_LOGE("module_task", "attempting to resume a dead task");
            return 1;
        }
        _desired_state = TaskState::running;

        if (!wait) {
            return 1;
        }

        // wait indefinitely
        while (_current_state != TaskState::paused) {}
        TBD_LOGV("module_task", "resumed task %s", _task.name());
        return 0;
    }


protected:
    // owned by owner of task
    std::atomic<TaskState> _desired_state;

    // for use in child process
    static void task_main_wrapper(void* param) {
        auto instance = reinterpret_cast<TaskModule*>(param);
        instance->task_main();
    }

    void task_main() {
        _current_state = TaskState::initializing;
        if (ModuleT::do_begin() != 0) {
            _current_state = TaskState::crashed;
            return;
        }

        TaskState target = _desired_state.load();
        while (target < TaskState::terminating) {
            if (target != TaskState::paused && target != TaskState::running) {
                _current_state = TaskState::ipc_error;
                return;
            }
            _current_state = target;

            if (_current_state == TaskState::paused) {
                Task::sleep(10);
                continue;
            }

            if (ModuleT::do_work() != 0) {
                _current_state = TaskState::crashed;
                return;
            }
        }

        _current_state = TaskState::terminating;
        if (ModuleT::do_cleanup() != 0) {
            _current_state = TaskState::crashed;
            return;
        }
        _current_state = TaskState::done;
    }

    // owned by task context
    Task _task;
    std::atomic<TaskState> _current_state;
};

}