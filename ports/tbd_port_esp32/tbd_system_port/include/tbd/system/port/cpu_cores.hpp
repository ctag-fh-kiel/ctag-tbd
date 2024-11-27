#include <tbd/system/common/cpu_cores.hpp>

#include "esp_cpu.h"


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
