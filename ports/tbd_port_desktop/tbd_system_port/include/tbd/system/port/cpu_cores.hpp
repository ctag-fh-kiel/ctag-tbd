#include <tbd/system/common/cpu_cores.hpp>

#include <chrono>

/** @brief ensure processing speed can handle n ops per second on ESP
 *
 *  @note: Relies on  CPU working at full speed since TBD should never have
 *         power saving features enabled.
 */
#define TBD_TIMEOUT_ENSURE_OPS_PER_SECOND(timeout_name, ops_per_second) \
tbd::system::TimeoutGuard timeout_name(std::chrono::seconds(1) / ops_per_second);

namespace tbd::system {

struct TimeoutGuard {
    TimeoutGuard(std::chrono::system_clock::duration max_duration)
        : _max_duration(max_duration), _start_time(std::chrono::high_resolution_clock::now())
    {

    }

    operator bool() const {
        return std::chrono::high_resolution_clock::now() - _start_time <= _max_duration;
    }


private:
    std::chrono::high_resolution_clock::duration _max_duration;
    std::chrono::high_resolution_clock::time_point _start_time;
};

}
