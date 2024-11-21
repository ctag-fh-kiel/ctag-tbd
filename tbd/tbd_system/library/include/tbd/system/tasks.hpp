#pragma once

#include <tbd/system/port/tasks.hpp>
#include <tbd/system/cpu_cores.hpp>


namespace tbd::system {

using task_handler_type = void(*)(void*);

template<class TaskT>
concept TaskType = requires(
    TaskT task, 
    CpuCore core_id,
    uint32_t _uint32_t, 
    uint8_t _uint8_t, 
    const char* _const_char_ptr,
    task_handler_type _task_func, 
    void* _void_ptr
) {
    { task.is_running() } -> std::same_as<bool>;
    { task.begin(_task_func, _void_ptr, core_id, _uint8_t) } -> std::same_as<void>;
    { task.begin(_task_func, _void_ptr, core_id) } -> std::same_as<void>;
    { task.begin(_task_func, _void_ptr) } -> std::same_as<void>;
    { TaskT::sleep(_uint32_t) } -> std::same_as<void>;
};


// check if Task provided py port satisfies requirements
static_assert(TaskType<Task>, "port task type does not fullfil requirements");

}
