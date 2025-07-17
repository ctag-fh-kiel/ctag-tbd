#pragma once

#include <cinttypes>


namespace tbd::serialization {

template<class MessageT>
constexpr int32_t max_message_size();

}