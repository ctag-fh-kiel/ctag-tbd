#pragma once

#include <tbd/system/port/tasks.hpp>


namespace tbd::system {

using task_handler_type = void(*)(void*);

template<class TaskT>
concept TaskType = requires(
    TaskT task, 
    uint32_t _uint32_t, 
    uint8_t _uint8_t, 
    const char* _const_char_ptr,
    task_handler_type _task_func, 
    void* _void_ptr
) {
    { task.begin(_task_func, _void_ptr, _uint8_t) } -> std::same_as<void>;
    { task.sleep(_uint32_t) } -> std::same_as<void>;
    { task.destroy() } -> std::same_as<void>;
};

// check if Task provided py port satisfies requirements
static_assert(TaskType<Task>);

}
