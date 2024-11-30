#pragma once

#include <cinttypes>


namespace tbd::api {

struct RestApiParams {
    uint16_t port;
};

struct RestApi {

    static void begin(const RestApiParams& rest_api_params = {.port = 2024});
    static void end();

};

}
