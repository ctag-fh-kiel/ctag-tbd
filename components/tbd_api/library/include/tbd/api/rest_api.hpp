#pragma once

#if !TBD_API_WIFI
    #error "RestApi module not available in config"
#endif

#include <tbd/api/common/rest_api.hpp>
#include <tbd/api/port/rest_api.hpp>

namespace tbd::api {

static_assert(RestApiType<RestApi, RestApiParams>, "RestApi type does not fullfill requirements");

}