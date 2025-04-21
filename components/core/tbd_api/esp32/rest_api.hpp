#pragma once

#include <cinttypes>


namespace tbd::api {

struct RestApiParams {};

struct RestApi {
    static void begin(const RestApiParams& rest_api_params = {});
    static void end();
};

}
