#pragma once

#include <concepts>
#include <cinttypes>


namespace tbd::audio {

template<class InputManagerT>
concept InputManagerType = requires(uint8_t** _uint8_ptr_ptr, float** _float_ptr_ptr) {
    // deviation from std::mutex: lock timeout
    { InputManagerT::Update(_uint8_ptr_ptr, _float_ptr_ptr) } -> std::same_as<void>;
};

}
