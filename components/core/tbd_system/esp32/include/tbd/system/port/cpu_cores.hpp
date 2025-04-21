#include <tbd/system/common/cpu_cores.hpp>

#include "esp_cpu.h"
#include "sdkconfig.h"

#ifndef CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ
    #error "no default ESP frequency dfined: undefined CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ"
#endif

/** @brief ensure processing speed can handle n ops per second on ESP
 *
 *  @note: Relies on  CPU working at full speed since TBD should never have
 *         power saving features enabled.
 */
#define TBD_TIMEOUT_ENSURE_OPS_PER_SECOND(timeout_name, ops_per_second) \
    tbd::system::TimeoutGuard timeout_name(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ * 1000000 / ops_per_second)

namespace tbd::system {

struct TimeoutGuard {
    TimeoutGuard(uint32_t max_cycles)
        : _max_cycles(max_cycles), _start_count(esp_cpu_get_cycle_count())
    {

    }

    operator bool() const { 
        return (esp_cpu_get_cycle_count() - _start_count) <= _max_cycles;
    }


private:
    uint32_t _max_cycles;
    esp_cpu_cycle_count_t _start_count;
};

}
