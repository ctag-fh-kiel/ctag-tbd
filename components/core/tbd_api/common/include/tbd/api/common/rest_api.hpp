#pragma once

#include <concepts>

namespace tbd::api {

template<class RestApiT, class RestApiParamsT>
concept RestApiType = requires(const RestApiParamsT& params) {
    { RestApiT::begin(params) } -> std::same_as<void>;
    { RestApiT::begin() } -> std::same_as<void>;
    { RestApiT::end() } -> std::same_as<void>;
};

}
