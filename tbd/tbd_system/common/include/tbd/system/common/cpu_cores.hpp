#pragma once

#include <cinttypes>
#include <type_traits>


namespace tbd::system {

enum class CpuCore: uint8_t {
    system = 0,
    audio = 1,
};

template<class CounterT>
concept TimeoutGuardType = 
    std::is_constructible_v<CounterT, uint32_t>
    && std::is_nothrow_destructible_v<CounterT>
    && requires(CounterT counter, uint32_t _uint32_t) 
{
    { !counter } -> std::same_as<bool>;
};


}
